#include "../include/bluetooth_ble.h"
#include <zephyr.h>
#include <kernel/thread_stack.h>
#include "../include/dijkstra.h"
#include "kernel.h"
#include <timing/timing.h>

/* Queues for message passing and processing */
K_MSGQ_DEFINE(common_received_packets_q,
        sizeof(struct net_buf_simple), 10, 4);


K_MSGQ_DEFINE(awaiting_ack,
        sizeof(ble_ack_info), 1, 4);
K_MSGQ_DEFINE(sending_ack_q,
        sizeof(ble_ack_info), 1, 4);

K_FIFO_DEFINE(common_packets_to_send_q);

/* Events for indicating if message was sent and scanned succesfully */
K_EVENT_DEFINE(ack_received);

K_MUTEX_DEFINE(ble_send_mutex);

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
        printk("Waiting for rx packet.. \n");
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
            //printk("Packet's final destination is node: %d\n", dst_mesh_id);

            // calculate next node from dijkstra algorithm
            //printk("Calculating Dijkstra's algorithm...\n");
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
                //printk("Next hop: %d\n", next_node_mesh_id);
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
        
        // hack to bypass auto prepended type 
        uint16_t timestamp = ble_add_packet_timestamp(
                &tx_packet->data.data[2]);
        uint8_t *extracted_data = &(tx_packet->data.data[2]);
        
        // create bt data 
        struct bt_data ad[] = {BT_DATA(common_self_mesh_id,
                extracted_data, 8)};
        
        // Add a flag indicating waiting on ACK from the receiver node
        ble_ack_info ack_info = {
            .node_id = next_node_mesh_id,
            .time_stamp = timestamp
        };
        
        err = k_msgq_put(&awaiting_ack, &ack_info, K_NO_WAIT);
        if(err){
            printk("ERROR: Failed to put into awaiting_ack q : %d.\n", err);
        }

        struct bt_le_adv_param adv_tx_params = BT_LE_ADV_PARAM_INIT(
                BT_LE_ADV_OPT_USE_IDENTITY,
                BT_GAP_ADV_FAST_INT_MIN_1,
                BT_GAP_ADV_FAST_INT_MAX_1,
                NULL);

        // wait until previous adv finished and advertise current msg
        k_mutex_lock(&ble_send_mutex, K_FOREVER);
        printk("Mutex unlocked for new msg adv.\n");
        printk("Advertising to node: %d \n", next_node_mesh_id);
        do{
            err = bt_le_adv_start(&adv_tx_params, 
            ad, ARRAY_SIZE(ad),
            NULL, 0);
            printk("ERROR: %d\n", err);
        }while(err);
        
        // Wait a moment and stop transmission so ack thread can send
        k_msleep(500);
        err = bt_le_adv_stop();
        k_mutex_unlock(&ble_send_mutex);
        
        /*
        uint32_t events;
        events = k_event_wait_all(&ack_received,
                BLE_ACK_RECEIVED_EVENT, true, K_MSEC(1000));
        if(events == 0){
            printk("ERROR: Node %d has not acknowledged! \n",
                    next_node_mesh_id);
            printk("Events: %X\n", ack_received.events); 
        }
        */

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

void ble_send_ack_thread_entry(void *unused1, void *unused2,  void *unused3){
    ble_ack_info ack_info;

    while(1){
        printk("Waiting for ack msg to send...\n");
        int err = k_msgq_get(&sending_ack_q, &ack_info, K_FOREVER);
        printk("Got ack message to send\n");
        if(err){
            printk("ERROR: Reading from ack msg queue failed.\n");
            return;
        }
        // all indexing here must be definedidx - 2 TODO fix this
        // Prep ack packet
        uint8_t ack_data[] = {
            0x00, 0x00, 0x00, 0x00, 0x00 ,0x00, 0x00, 0x00
        };
        ack_data[MSG_TYPE_IDX - 2] = MSG_TYPE_ACK;
        ack_data[DST_ADDR_IDX - 2] = 0xFF; //whatever, ignored
        ack_data[RCV_ADDR_IDX - 2] = ack_info.node_id; // node to ack to
        ack_data[TIME_STAMP_UPPER_IDX - 2] = 0xFF00 & ack_info.time_stamp;
        ack_data[TIME_STAMP_LOWER_IDX - 2] = 0x00FF & ack_info.time_stamp;

        struct bt_data ack_ad[] = {BT_DATA(common_self_mesh_id, 
                ack_data, ARRAY_SIZE(ack_data))};

        struct bt_le_adv_param adv_ack_params = BT_LE_ADV_PARAM_INIT(
                BT_LE_ADV_OPT_USE_IDENTITY,
                BT_GAP_ADV_FAST_INT_MIN_1,
                BT_GAP_ADV_FAST_INT_MAX_1,
                NULL);
        k_mutex_lock(&ble_send_mutex, K_FOREVER);
        do{
            err = bt_le_adv_start(&adv_ack_params, 
            ack_ad, ARRAY_SIZE(ack_ad),
            NULL, 0);
            printk("Err in ack %d \n", err);
        }while(err);
        
		k_msleep(200);
        err = bt_le_adv_stop();
        k_mutex_unlock(&ble_send_mutex);
        
        printk("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n");
        printk("ACK SENT to node %d.\n", ack_info.node_id);
        printk("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n");

        if(err){
            printk("ERROR: Advertising failed to stop. \n");
            return;
        }
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
        
    //printk("######################################################\n");

    // check if receiver 
    bool is_receiver = ble_is_receiver(buf, common_self_mesh_id);

    if(is_receiver){
        printk("Received data from node with address: %s\n", addr_str);
        printk("Data: %s\n", data);
        
        int err;
        uint8_t msg_type = buf->data[MSG_TYPE_IDX];
        switch(msg_type){
            case MSG_TYPE_DATA:
                {
                    printk("RECEIVED DATA MSG from %d.\n", 
                            buf->data[SENDER_ID_IDX]);
                    // Do not send ack if msg is on broadcast addr
                    if(!(buf->data[RCV_ADDR_IDX] == BROADCAST_ADDR)){
                        // Queue ack for the msg sender
                        ble_ack_info ack_info = {
                            .node_id = buf->data[SENDER_ID_IDX],
                            .time_stamp = ble_get_packet_timestamp(buf->data)
                        };
                        err = k_msgq_put(&sending_ack_q, 
                                &ack_info, K_NO_WAIT);
                        if(err){
                            printk("ERROR: Failed to put to ack \
                                    send queue\n");
                            return;
                        }
                    }
                    
                    // Queue message to process further
                    err = k_msgq_put(
                            &common_received_packets_q, 
                            buf, K_NO_WAIT);
                    if(err){ 
                        printk("Error queue put: %d, queue purged\n", err);
                        k_msgq_purge(&common_received_packets_q);
                    }
                }
                break;

            case MSG_TYPE_ACK:
                {
                    // Check if message's header content is correct 
                    // with awaited
                    ble_ack_info a_info;
                    err = k_msgq_peek(&awaiting_ack, &a_info);
                    if(err){
                        printk("ERROR: No info about awaited ack!\n");
                        return;
                    }
                    printk("RECEIVED ACK MSG FROM: %d\n", 
                            buf->data[SENDER_ID_IDX]);
                    bool correct_id = 
                        a_info.node_id == buf->data[RCV_ADDR_IDX];
                    uint16_t timestamp16 = ble_get_packet_timestamp(
                            buf->data);
                    bool correct_timestamp = timestamp16 == a_info.time_stamp;
                    if(correct_id && correct_timestamp){
                        k_event_post(
                                &ack_received,
                                BLE_ACK_RECEIVED_EVENT); 
                    }
                    break;
                }
                
            case MSG_TYPE_ROUTING_TAB:
                printk("ERROR: Not implemented.\n");
                break;
        }
        
    }
    //printk("######################################################\n");
}


void ble_scanned(struct bt_le_ext_adv *adv,
        struct bt_le_ext_adv_scanned_info *info){
    // adv - advertising set obj
    // info - address adn type
    char addr_str[BT_ADDR_LE_STR_LEN];
    bt_addr_le_to_str(info->addr, addr_str, sizeof(addr_str));
    
    printk("Sent scan data to the node with addr: %s\n", addr_str);
}


void ble_sent(struct bt_le_ext_adv *adv, 
        struct bt_le_ext_adv_sent_info *info){
    // Fired after a num of adv ev or timeout, no need to count
    printk("Sent adv data \n");
}

/* Utility functions */
bool ble_is_receiver(struct net_buf_simple *buf,uint8_t common_self_mesh_id){
    bool rec = buf->data[RCV_ADDR_IDX] == BROADCAST_ADDR || 
            buf->data[RCV_ADDR_IDX] == common_self_mesh_id;
    return rec;
}


uint16_t ble_add_packet_timestamp(uint8_t data[]){
    uint8_t timestamp_lower, timestamp_upper;
    uint32_t cycles32 = k_cycle_get_32();

    // Get only lower 16 bits
    timestamp_lower = 0x00FF & cycles32;
    timestamp_upper = (0xFF00 & cycles32) >> 8;
    data[TIME_STAMP_UPPER_IDX] = timestamp_upper;
    data[TIME_STAMP_LOWER_IDX] = timestamp_lower;
    return cycles32 & 0xFFFF;
}


uint16_t ble_get_packet_timestamp(uint8_t data[]){
    uint16_t timestamp = (data[TIME_STAMP_UPPER_IDX] << 8) | 
        data[TIME_STAMP_LOWER_IDX];
    return timestamp;
}


