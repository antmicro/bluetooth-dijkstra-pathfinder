#include "../include/bluetooth_ble.h"
#include <zephyr.h>
#include <kernel/thread_stack.h>
#include "../include/dijkstra.h"
#include <timing/timing.h>

/* Queues for message passing and processing */
K_MSGQ_DEFINE(common_received_packets_q,
        sizeof(struct net_buf_simple), 10, 4);
K_FIFO_DEFINE(common_packets_to_send_q);

/* Events for indicating if message was sent and scanned succesfully */
K_EVENT_DEFINE(ble_sending_completed);


/* Setup functions */
void ble_scan_setup(struct bt_le_scan_param *scan_params){
    *(scan_params) = (struct bt_le_scan_param){
		.type       = BT_LE_SCAN_TYPE_PASSIVE,  
		.options    = BT_LE_SCAN_OPT_NONE,//BT_LE_SCAN_OPT_CODED | 
		.interval   = 0x0060, //BT_GAP_SCAN_FAST_INTERVAL,
		.window     = 0x0060//BT_GAP_SCAN_FAST_WINDOW,
	};
    
    // register a callback for packet reception
    static struct bt_le_scan_cb scan_callbacks = {
        .recv = bt_msg_received_cb,
    };
    bt_le_scan_cb_register(&scan_callbacks);
}


/* Thread entries */
void create_packet_thread_entry(struct node_t *graph){ 
    struct net_buf_simple buf; 
    uint8_t dst_mesh_id;
    int err;
         
    while(1){
        err = k_msgq_get(&common_received_packets_q, &buf, K_FOREVER);
               
        if(err){
            printk("Error reading from queue: %d \n!\n", err);
        }
        else{
            // retrieve dst node from data packet 
            dst_mesh_id = buf.data[DST_ADDR_IDX];

            if(dst_mesh_id == common_self_mesh_id){
                printk("FINAL DESTINATION REACHED!\n");
                
                // potential processing

                continue;
            }
            printk("Packet's final destination is node: %d\n", dst_mesh_id);

            // calculate next node from dijkstra algorithm
            printk("Calculating Dijkstra's algorithm...\n");
            int next_node_mesh_id = dijkstra_shortest_path(
                    graph, MAX_MESH_SIZE,
                    common_self_mesh_id, 
                    dst_mesh_id);

            if(next_node_mesh_id < 0){
                printk("Dijkstra algorithm failed\n");
            }
            else{
                // if all is good, create a packet and queue it to tx 
                buf.data[RCV_ADDR_IDX] = next_node_mesh_id; 
                printk("Next hop: %d\n", next_node_mesh_id);
                struct ble_tx_packet_data tx_packet = {
                    .data = buf
                };
                k_fifo_put(&common_packets_to_send_q, &tx_packet);
            }
        }
    }
}


void ble_send_packet_thread_entry(struct node_t *graph, 
        struct bt_le_scan_param *params){
    struct ble_tx_packet_data *tx_packet;
    
    while(1){
        printk("waiting for tx_packet....\n");
        tx_packet = k_fifo_get(&common_packets_to_send_q, K_FOREVER);

        if(!tx_packet){
            printk("ERROR: could not get tx_packet from queue.");
        }

        int err;
        uint8_t next_node_mesh_id = tx_packet->data.data[RCV_ADDR_IDX];
        
        // hack to bypass auto prepended type / header   
        uint8_t *extracted_data = &(tx_packet->data.data[2]);

        // create bt data 
        struct bt_data ad[] = {BT_DATA(common_self_mesh_id,
                extracted_data, 8)};

        printk("######################################################\n");
        printk("Advertising to node: %d\n", next_node_mesh_id);
        printk("######################################################\n");
        
        // wait until previous adv finished and advertise current msg
        do{
            err = bt_le_adv_start(BT_LE_ADV_NCONN_IDENTITY, 
            ad, ARRAY_SIZE(ad),
            NULL, 0);
            printk("Sending status: %d \n", err);
        }while(err);
        
        uint32_t events;

        events = k_event_wait_all(&ble_sending_completed,
                BLE_SCANNED_EVENT, true, K_MSEC(1000));
        if(events == 0){
            printk("ERROR: Node %d has not scanned for data! \n",
                    next_node_mesh_id);
            printk("Events: %X\n", ble_sending_completed.events); 
        }

        /*
         * Calculate new tentative_distance on the basis of the number 
         * of unsuccessful transmission sequences. Update the number of 
         * unsuccessful transmissions and set the new td.
         * */
        /*
        node_update_missed_transmissions(
                    &graph[tx_packet->next_node_mesh_id], events);
        uint8_t new_td = calc_td_from_missed_transmissions(
                    graph[tx_packet->next_node_mesh_id].missed_transmissions);
        graph_set_distance(graph,
                    common_self_mesh_id, tx_packet->next_node_mesh_id,
                    new_td);
        
        // If transmission unsuccessful, put packet to fifo again    
        if(!events) 
            k_fifo_put(&common_packets_to_send_q, tx_packet);
        */
    }
}


/* Callbacks */
void bt_msg_received_cb(const struct bt_le_scan_recv_info *info,
              struct net_buf_simple *buf){
    char addr_str[BT_ADDR_LE_STR_LEN];
    char data[31];
    
    // formatting 
    bin2hex(buf->data, buf->len, data, sizeof(data));
    bt_addr_le_to_str(info->addr, addr_str, sizeof(addr_str));
        
    printk("######################################################\n");
    printk("Received data from node with address: %s\n", addr_str);
    printk("Data: %s\n", data);

    // check if receiver 
    bool is_receiver = ble_is_receiver(buf, common_self_mesh_id);
    
    // add to queue
    if(is_receiver){
        printk("Current node is the receiver of this msg: %X \n", 
                buf->data[RCV_ADDR_IDX]);
        int err = k_msgq_put(&common_received_packets_q, buf, K_NO_WAIT);
        if(err){ 
            printk("Error queue put: %d, queue purged\n", err);
            k_msgq_purge(&common_received_packets_q);
        }
    }
    printk("######################################################\n");
}


void ble_scanned(struct bt_le_ext_adv *adv,
        struct bt_le_ext_adv_scanned_info *info){
    // adv - advertising set obj
    // info - address adn type
    char addr_str[BT_ADDR_LE_STR_LEN];
    bt_addr_le_to_str(info->addr, addr_str, sizeof(addr_str));
    
    printk("Sent scan data to the node with addr: %s\n", addr_str);
    k_event_post(&ble_sending_completed, BLE_SCANNED_EVENT); 
}


void ble_sent(struct bt_le_ext_adv *adv, 
        struct bt_le_ext_adv_sent_info *info){
    // Fired after a num of adv ev or timeout, no need to count
    printk("Sent adv data \n");
}

/* Utility functions */
bool ble_is_receiver(struct net_buf_simple *buf,uint8_t common_self_mesh_id){
    bool rec = buf->data[RCV_ADDR_IDX] == BROADCAST_ADDR || 
            buf->data[DST_ADDR_IDX] == ROUTING_TABLE_ID ||
            buf->data[RCV_ADDR_IDX] == common_self_mesh_id;
    
    return rec;
}


