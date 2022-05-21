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
    // directed messaging scan params
    *(scan_params) = (struct bt_le_scan_param){
		.type       = BT_LE_SCAN_TYPE_ACTIVE,  
		.options    = 0,//BT_LE_SCAN_OPT_CODED | 
		.interval   = 0x0060, //BT_GAP_SCAN_FAST_INTERVAL,
		.window     = 0x0060//BT_GAP_SCAN_FAST_WINDOW,
	};
    
    // register a callback for packet reception
    static struct bt_le_scan_cb scan_callbacks = {
        .recv = bt_direct_msg_received_cb,
    };
    bt_le_scan_cb_register(&scan_callbacks);
}


void ble_adv_sets_setup(struct node_t *graph, struct bt_le_ext_adv **adv_set){ 
    uint32_t ext_adv_aptions = 
        BT_LE_ADV_OPT_EXT_ADV
        + BT_LE_ADV_OPT_DIR_MODE_LOW_DUTY
        + BT_LE_ADV_OPT_SCANNABLE           // Add for active scanning
        + BT_LE_ADV_OPT_NOTIFY_SCAN_REQ;
  
    struct bt_le_adv_param params = {
        .id = 0x0,
        .options = ext_adv_aptions,
        .interval_min = BT_GAP_ADV_FAST_INT_MIN_1, // 30ms  
        .interval_max = BT_GAP_ADV_FAST_INT_MAX_1, // 60ms 
    };

    static struct bt_le_ext_adv_cb adv_callbacks = {
        .sent = ble_sent,
        .scanned = ble_scanned
    };
    
    // TODO: handle unreserved nodes !!!
    // get addr from string to proper format
    for(uint8_t i = 0; i < MAX_MESH_SIZE; i++){
        int err; 
        bt_addr_t temp_ble_addr;
        
        err = bt_addr_from_str(graph[i].addr_bt_le, &temp_ble_addr);
        if(err){
            printk("Incorrect BLE addr string! Node mesh addr: %d err: %d\n",
                    err, graph[i].addr);
            if(!graph[i].reserved){
                printk("Node %d not reserved\n", graph[i].addr);
            }
        }
            
        // set up receiver address 
        bt_addr_le_t receiver_addr = {
            .type = BT_ADDR_LE_RANDOM,
            .a = temp_ble_addr
        };

        params.peer = &receiver_addr;

        // 2nd param is callbacks struct for adv 
        err = bt_le_ext_adv_create(&params, &adv_callbacks, &adv_set[i]);
        if(err){
            printk("Error creating advertising set! err: %d\n", err);
        }
    }

}


/* Utility functions */
void get_mesh_id_from_data(struct net_buf_simple *buf,
        uint8_t *mesh_id){
    // addr of dst node in mesh addr form is in first byte
    *mesh_id = buf->data[2]; 
}


/* Thread entries */
void create_packet_thread_entry(struct node_t *graph){ 
    struct net_buf_simple buf; 
    uint8_t dst_mesh_id;
    int err;

    // Timing setup 
    timing_t start_time, receive_time;
    uint64_t receive_time_in_ns;
    uint64_t cycles; 
    
    // start time measurement
    timing_init();
    timing_start();
    start_time = timing_counter_get();
     
    while(1){
        err = k_msgq_get(&common_received_packets_q, &buf, K_FOREVER);
               
        if(err){
            printk("Error reading from queue: %d \n!\n", err);
        }
        else{
            // retrieve dst node from data packet (3rd byte)
            get_mesh_id_from_data(&buf, &dst_mesh_id);

            if(dst_mesh_id == common_self_mesh_id){
                printk("FINAL DESTINATION REACHED!\n");
               
                // copy time stamp to a buffer
                uint64_t packet_send_time = 0;
                memcpy(&packet_send_time, &buf.data[3],
                        sizeof(packet_send_time));
                
                // get current time in ns
                receive_time = timing_counter_get(); 
                cycles = timing_cycles_get(&start_time, &receive_time);
                receive_time_in_ns = timing_cycles_to_ns(cycles); 
                
                // subtract to get travel time 
                uint64_t travel_time = receive_time_in_ns - packet_send_time;
                //printk("Travel time of a packet was: %llu, %llu\n", travel_time, cycles);
                continue;
            }
            printk("Packet's final destination is node: %d\n", dst_mesh_id);

            // calculate next node from dijkstra algorithm
            printk("Calculating Dijkstra's algorithm...\n");
            int next_node_mesh_id = dijkstra_shortest_path(graph, MAX_MESH_SIZE,
                    common_self_mesh_id, dst_mesh_id);

            if(next_node_mesh_id < 0){
                printk("Dijkstra algorithm failed\n");
            }
            else{
                // if all is good, create a packet and queue it to tx 
                printk("Next hop: %d\n", next_node_mesh_id);
                
                struct ble_tx_packet_data tx_packet = {
                    .next_node_mesh_id = next_node_mesh_id,
                    .data = buf
                };
                k_fifo_put(&common_packets_to_send_q, &tx_packet);
            }
        }
    }
}


void ble_send_packet_thread_entry(struct node_t *graph, 
        struct bt_le_scan_param *params){
    /* Bluetooth direct adv setup*/
    struct bt_le_ext_adv *adv_set[MAX_MESH_SIZE];// TODO this is problem
    ble_adv_sets_setup(graph, adv_set);
    
    struct ble_tx_packet_data *tx_packet;

    while(1){
        int err;
       
        printk("waiting for tx_packet....\n");
        tx_packet = k_fifo_get(&common_packets_to_send_q, K_FOREVER);
        
        // pick adv set proper for next node mesh id 
        struct bt_le_ext_adv *current_set = adv_set[tx_packet->next_node_mesh_id];
        
        // hack to bypass auto appended type / header  
        uint8_t *extracted_data = &(tx_packet->data.data[2]);

        // create bt data 
        struct bt_data ad[] = {BT_DATA(common_self_mesh_id, extracted_data, 8)};

        // to whatever, for identification of ad message only
        //uint8_t temp_data[] = {0x09, 0x09, 0x09, 0x09, 0x09 ,0x09, 0x09, 0x09}; 
        //struct bt_data sd[] = {BT_DATA(common_self_mesh_id, temp_data, 8)};

        // set data to one from received packet  
        err = bt_le_ext_adv_set_data(current_set,
                NULL, 0, 
                ad, ARRAY_SIZE(ad));
        if(err){
            printk("Error setting advertisement data: %d\n", err);
        }

        printk("######################################################\n");
        printk("Advertising to node: %d\n", tx_packet->next_node_mesh_id);
        printk("######################################################\n");
        
        // wait until previous adv finished and advertise current msg
        do{
            err = bt_le_ext_adv_start(
                    current_set, 
                    BT_LE_EXT_ADV_START_PARAM(0, 5)); 
        }while(err);

        uint32_t events;
        events = k_event_wait_all(&ble_sending_completed,
                BLE_SCANNED_EVENT, true, K_MSEC(1000));
        if(events == 0){
            printk("ERROR: Node %d has not scanned for data! \n",
                    tx_packet->next_node_mesh_id);
            printk("Events: %X\n", ble_sending_completed.events); 
            
            // Set new dist between self and next node as infinity (255)
            // basically disabling node from communiaction
            graph_update_distance(graph,
                    common_self_mesh_id, tx_packet->next_node_mesh_id,
                    INF);
            // Update routine so its not so binary. 
            
            // Put the packet again into fifo to try to send to another 
            // node after adjusting the graph
            k_fifo_put(&common_packets_to_send_q, tx_packet);
        }
    }
}


/* Callbacks */
void bt_direct_msg_received_cb(const struct bt_le_scan_recv_info *info,
              struct net_buf_simple *buf){
    char addr_str[BT_ADDR_LE_STR_LEN];
    char data_str[31];
    
    // formatting 
    bin2hex(buf->data, buf->len, data_str, sizeof(data_str));
    bt_addr_le_to_str(info->addr, addr_str, sizeof(addr_str));

    // print
    printk("######################################################\n");
    printk("Received data from node with address: %s\n", addr_str);
    printk("Data: %s\n", data_str);
    printk("######################################################\n");
    
    // add to queue  
    int err = k_msgq_put(&common_received_packets_q, buf, K_NO_WAIT);
    if(err){ // TODO: change this purge, this is little extreme 
        printk("Error queue put: %d, queue purged\n", err);
        k_msgq_purge(&common_received_packets_q);
    }
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

