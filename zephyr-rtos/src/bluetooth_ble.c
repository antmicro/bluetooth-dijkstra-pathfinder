#include "../include/bluetooth_ble.h"
#include <zephyr.h>
#include <kernel/thread_stack.h>
#include "../include/dijkstra.h"
#include "kernel.h"
#include <timing/timing.h>
#include <assert.h>

/* Queues for message passing and processing */
// Holds single instance of info about awaited ack message
K_MSGQ_DEFINE(awaiting_ack,
        sizeof(ble_sender_info), 1, 4);

// Queues for threads prepping messages
K_MSGQ_DEFINE(ack_receivers_q,
        sizeof(ble_sender_info), 10, 4);
K_MSGQ_DEFINE(data_packets_to_send_q,
        BLE_MSG_LEN, 10, 4);

// Queue for messages to send
K_MSGQ_DEFINE(ready_packets_to_send_q, 
        BLE_MSG_LEN, 10, 4); 

/* Events for indicating if message was sent and scanned succesfully */
K_EVENT_DEFINE(ack_received);

K_MUTEX_DEFINE(ble_send_mutex);

// Static initialization of circular buffer with recently received packets
RCV_PKTS_DEFINE(rcv_pkts_circular_buffer, 10);

/* Setup functions */
void ble_scan_setup(struct bt_le_scan_param *scan_params){
    *(scan_params) = (struct bt_le_scan_param){
		.type       = BT_LE_SCAN_TYPE_PASSIVE,  
		.options    = BT_LE_SCAN_OPT_NONE,//BT_LE_SCAN_OPT_CODED | 
		.interval   =  0x0060, 
		.window     =  0x0060 
	};
    
    // register a callback for packet reception
    static struct bt_le_scan_cb scan_callbacks = {
        .recv = bt_msg_received_cb,
    };
    bt_le_scan_cb_register(&scan_callbacks);
}


/* Thread entries */
void ble_prep_data_packet_thread_entry(
        struct node_t *graph) {
    while(1){
        uint8_t ble_data[BLE_MSG_LEN];
        int err = k_msgq_get(&data_packets_to_send_q, ble_data, K_FOREVER);
        if(err){
            printk("ERROR: reading from queue: %d \n!\n", err);
            return;
        }
        
        // If current node is destination node of the packet, do not send further
        uint8_t dst_mesh_id = ble_data[DST_ADDR_IDX];
        if(dst_mesh_id == common_self_mesh_id) {
            printk("FINAL DESTINATION REACHED\n");
            continue;
        } 
        
        // Calculate shortest path with Dijkstra algorithm
        uint8_t next_node_mesh_id = dijkstra_shortest_path(graph, MAX_MESH_SIZE,
                common_self_mesh_id, dst_mesh_id);
        if(next_node_mesh_id < 0) {
            printk("ERROR: Dijkstra algorithm failed.\n");
            return;
        }
        
        // Set self as a sender 
        ble_data[SENDER_ID_IDX] = common_self_mesh_id;

        // Set the next node in the packet header
        ble_data[RCV_ADDR_IDX] = next_node_mesh_id;
        
        // Pass the message to sending thread
        err = k_msgq_put(&ready_packets_to_send_q, ble_data, K_NO_WAIT);
        if(err) {
            printk("ERROR: problem putting data packet to send queue: %d\n", err);
            return;
        }
    }
}


void ble_prep_ack_thread_entry(
        void *unused1,
        void *unused2,
        void *unused3) {
    ble_sender_info ack_info;
    while(1){
        int err = k_msgq_get(&ack_receivers_q, &ack_info, K_FOREVER);
        if(err){
            printk("ERROR: Reading from ack msg queue failed.\n");
            return;
        }
        // Prep ack packet
        uint8_t ack_data[] = {
            0x05, 0x05, 0x05, 0x05, 0x05 ,0x05, 0x05, 0x05
        };
        ack_data[SENDER_ID_IDX] = common_self_mesh_id;
        ack_data[MSG_TYPE_IDX] = MSG_TYPE_ACK;
        ack_data[DST_ADDR_IDX] = 0xFF; //whatever, ignored
        ack_data[RCV_ADDR_IDX] = ack_info.node_id; // node to ack to
        ack_data[TIME_STAMP_MSB_IDX] = (0xFF00 & ack_info.time_stamp) >> 8;
        ack_data[TIME_STAMP_LSB_IDX] = 0x00FF & ack_info.time_stamp;
        
        err = k_msgq_put(&ready_packets_to_send_q, ack_data, K_NO_WAIT);
        if(err) {
            printk("Error putting an ack data to send queue: %d\n.", err);
            return;
        }
    }
}


void ble_send_packet_thread_entry(
        struct node_t *graph, 
        struct bt_le_scan_param *params
        ) {
    int err;
    // Initialization  
    static uint8_t ad[BLE_MSG_LEN]; 
    static struct bt_data ad_arr[] = {
            BT_DATA(0xAA, ad, BLE_MSG_LEN) 
    };
    // Replace 0xAA with self mesh id 
    ad_arr[0].type = common_self_mesh_id;

    while(1){
        err = k_msgq_get(&ready_packets_to_send_q, &ad, K_FOREVER);
        if(err) {
            printk("ERROR: could not get packet to send from a queue.\n");
            return;
        }
        printk("Sending a message.\n");
    
        // If sending data, add flag indicating waiting for ack and modify time
        bool is_data_msg = ad[MSG_TYPE_IDX] == MSG_TYPE_DATA;
        if(is_data_msg) {
            uint16_t timestamp = ble_add_packet_timestamp(ad);
            uint8_t next_node_mesh_id = ad[RCV_ADDR_IDX];
            ble_sender_info ack_info = {
                .node_id = next_node_mesh_id,
                .time_stamp = timestamp,
                .msg_type = 0x0 // Unused here
            };
            printk("Awaiting for ack from node %d and with timestamp: %d\n",
                    next_node_mesh_id, timestamp);
            err = k_msgq_put(&awaiting_ack, &ack_info, K_NO_WAIT);
            if(err){
                printk("ERROR: Failed to put into awaiting_ack q : %d.\n", err);
                return;
            }
        }
        
        // Prep adv params
        static struct bt_le_adv_param adv_tx_params = BT_LE_ADV_PARAM_INIT(
                BT_LE_ADV_OPT_USE_IDENTITY,
                BT_GAP_ADV_FAST_INT_MIN_1,
                BT_GAP_ADV_FAST_INT_MAX_1,
                NULL);
        
        err = bt_le_adv_start(&adv_tx_params, 
        ad_arr, ARRAY_SIZE(ad_arr), 
        NULL, 0);
        if(err) {
            printk("ERROR: could not start advertising : %d\n", err);
            return;
        }

        // Wait for ack, returned value is ignored if ack is not required
        bool got_ack = ble_wait_for_ack(300);

        err = bt_le_adv_stop();
        if(err) {
            printk("ERROR: Failed to stop advertising %d.\n", err);
            return;
        }
        printk("Finished advertising.\n");
        
        // Take back previously set awaiting ack flag for data msg
        if(is_data_msg) {
            err = k_msgq_get(&awaiting_ack, NULL, K_MSEC(50));
            if(err) {
                printk("ERROR: Could not take down awaiting_ack flag: %d\n", err);
                return;
            }
        
            // Calculate new tentative_distance on the basis of the number 
            // of unsuccessful transmission sequences. Update the number of 
            // unsuccessful transmissions and set the new td.
             
            // TODO: Ugly, beautify this
            node_update_missed_transmissions(
                        &graph[ad[RCV_ADDR_IDX]], got_ack);
            uint8_t new_td = calc_td_from_missed_transmissions(
                        graph[ad[RCV_ADDR_IDX]].missed_transmissions);
            graph_set_distance(graph,
                        common_self_mesh_id, ad[RCV_ADDR_IDX], new_td);

            // Put the message again into the queue and try to send it again
            if(!got_ack) {
                printk("Did not receive ACK.\n");
                k_msgq_put(&data_packets_to_send_q, ad, K_NO_WAIT);
            }
        }
    }
}


/* Callbacks */
void bt_msg_received_cb(const struct bt_le_scan_recv_info *info,
              struct net_buf_simple *buf){
    char addr_str[BT_ADDR_LE_STR_LEN];
    char data[31];
    
    // Check if circular buffer should be popped 
    static int64_t prev_t = 0; 
    int64_t current_t = k_uptime_get();
    int64_t delta = current_t - prev_t;
    if(delta > CB_POP_TIME_MS) { // TODO: Adjust timing of this operation 
        rcv_pkts_cb_pop(&rcv_pkts_circular_buffer);
    }
    prev_t = current_t;

    // formatting 
    bin2hex(buf->data, buf->len, data, sizeof(data));
    bt_addr_le_to_str(info->addr, addr_str, sizeof(addr_str));
        
    printk("Received data from node with address: %s\n", addr_str);
    printk("Data: %s\n", data);
    
    // Strip the buffer into simple byte array
    uint8_t ble_data[BLE_MSG_LEN]; 
    memcpy(ble_data, &(buf->data[2]), BLE_MSG_LEN);

    // Check if is receiver, proceed if yes
    if(ble_is_receiver(ble_data, common_self_mesh_id)){
        int err;
        uint8_t msg_type = ble_data[MSG_TYPE_IDX];
        
        // Extract sender id, timestamp and msg type to determine processing 
        // path and check if it is a duplicate 
        ble_sender_info sender_info = {
                .node_id = ble_data[SENDER_ID_IDX],
                .time_stamp = ble_get_packet_timestamp(ble_data),
                .msg_type = msg_type
        };
        
        // If packet was not received before, process it 
        if(!rcv_pkts_cb_is_in_cb(
                    &rcv_pkts_circular_buffer, &sender_info)) {
            // Also add it to the recently received packets memory 
            rcv_pkts_cb_push(&rcv_pkts_circular_buffer, &sender_info);
            switch(msg_type){
                case MSG_TYPE_DATA:
                    {
                        printk("RECEIVED NEW DATA MSG from %d.\n", 
                                ble_data[SENDER_ID_IDX]);
                        
                        // Do not send ack if msg is on broadcast addr
                        if(!(ble_data[RCV_ADDR_IDX] == BROADCAST_ADDR)){
                            // Queue ack for the msg sender
                            err = k_msgq_put(&ack_receivers_q, 
                                    &sender_info, K_NO_WAIT);
                            if(err){
                                printk("ERROR: Failed to put to ack \
                                        send queue\n");
                                return;
                            }
                        }
                        
                        // Queue message to process further
                        err = k_msgq_put(
                                &data_packets_to_send_q, 
                                ble_data, K_NO_WAIT);
                        if(err){ 
                            printk("Error queue put: %d, queue purged\n", err);
                            k_msgq_purge(&data_packets_to_send_q);
                            return;
                        }
                    }
                    break;

                case MSG_TYPE_ACK:
                    {
                        // Check if message's header content is correct 
                        // with awaited
                        ble_sender_info a_info;
                        err = k_msgq_peek(&awaiting_ack, &a_info);
                        if(err){
                            printk("ERROR: No info about awaited ack!\n");
                            return;
                        }

                        bool correct_id = a_info.node_id == ble_data[SENDER_ID_IDX];
                        uint16_t timestamp16 = sender_info.time_stamp;
                        bool correct_timestamp = timestamp16 == a_info.time_stamp;
                        if(correct_id && correct_timestamp){
                            printk("RECEIVED ACK MSG FROM: %d\n", 
                                ble_data[SENDER_ID_IDX]);
                            
                            // Wake up sending thread so it can stop transmission
                            // as ack was already received
                            k_wakeup(send_packet_thread_id);
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
    }
}


/* Utility functions */
void rcv_pkts_cb_push(rcv_pkts_cb *cb, ble_sender_info *item) {
    *(cb->head) = *item;
    cb->head++;
    
    // Increase count
    cb->count++;
    if(cb->head == cb->buff_end) { // we dont write to end, its not valid 
        // In next call, write to the beginning of the buffer 
        cb->head = cb->buff_start;
        
        // This means, discard the first element -> shift tail if it points to 
        // that element
        if(cb->tail == cb->buff_start) cb->tail++;
        
    }
    // If head catches tail, shift tail and put it at the start if relapse
    // also decrease count, as one element was added with the cost of another 
    if(cb->head == cb->tail) {
        printk("Hi\n");
        cb->tail++;
        cb->count--;
        if(cb->tail == cb->buff_end) cb->tail = cb->buff_start;
    }
    // Test
    assert(cb->count <= cb->capacity);
}


void rcv_pkts_cb_pop(rcv_pkts_cb *cb) {
    if(cb->count > 0) {
        cb->tail++;
        if(cb->tail == cb->buff_end) cb->tail = cb->buff_start;
        cb->count--;
    }
}


bool rcv_pkts_cb_is_in_cb(rcv_pkts_cb *cb, ble_sender_info *item) {
    ble_sender_info *ptr = cb->tail;
    while(ptr != cb->head) {
        if(
                item->node_id == ptr->node_id && 
                item->time_stamp == ptr->time_stamp &&
                item->msg_type == ptr->msg_type) {
            return true;        
        }
        ptr++;
        if(ptr == cb->buff_end) ptr = cb->buff_start;
    }
    return false;
}


bool ble_is_receiver(uint8_t data[],uint8_t common_self_mesh_id){
    bool rec = data[RCV_ADDR_IDX] == BROADCAST_ADDR || 
            data[RCV_ADDR_IDX] == common_self_mesh_id;
    return rec;
}


uint16_t ble_add_packet_timestamp(uint8_t data[]){
    uint8_t timestamp_lower, timestamp_upper;
    uint32_t cycles32 = k_cycle_get_32();

    // Get only lower 16 bits
    timestamp_lower = 0x00FF & cycles32;
    timestamp_upper = (0xFF00 & cycles32) >> 8;
    data[TIME_STAMP_MSB_IDX] = timestamp_upper;
    data[TIME_STAMP_LSB_IDX] = timestamp_lower;
    return cycles32 & 0xFFFF;
}


uint16_t ble_get_packet_timestamp(uint8_t data[]){
    uint16_t timestamp = (data[TIME_STAMP_MSB_IDX] << 8) | 
        data[TIME_STAMP_LSB_IDX];
    return timestamp;
}

bool ble_wait_for_ack(int32_t timeout_ms) {
    int32_t time_remaining = k_msleep(timeout_ms);
    return time_remaining > 0;
}
