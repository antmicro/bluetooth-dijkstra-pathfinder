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

// Queues for threads sending messages
K_MSGQ_DEFINE(ack_receivers_q,
        sizeof(ble_sender_info), 10, 4);
K_MSGQ_DEFINE(data_packets_to_send_q,
        sizeof(struct ble_packet_info), 10, 4);
K_MSGQ_DEFINE(rtr_packets_to_send_q,
        BLE_RTR_MSG_LEN, 10, 4);

// Queue for messages to send
K_MSGQ_DEFINE(ready_packets_to_send_q, 
        BLE_DATA_MSG_LEN, 10, 4); 

K_MUTEX_DEFINE(ble_send_mutex);

K_TIMER_DEFINE(add_self_to_rtr_queue_timer, add_self_to_rtr_queue, NULL);
K_TIMER_DEFINE(pop_from_cb_timer, add_self_to_rtr_queue, NULL);

// Static initialization of circular buffer with recently received packets
RCV_PKTS_DEFINE(rcv_pkts_circular_buffer, 20);

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
void ble_send_data_packet_thread_entry(
        struct node_t *graph) {
    while(1){
        // Reserved memory for data and array for sending by BLE
        static struct ble_packet_info pkt_info;
        static struct bt_data ad_arr[] = {
            BT_DATA(0xAA, pkt_info.ble_data, BLE_DATA_MSG_LEN) 
        };

        // Get the packet content from the queue
        int err = k_msgq_get(&data_packets_to_send_q, &pkt_info, K_FOREVER);
        if(err){
            printk("ERROR: reading from queue: %d \n!\n", err);
            return;
        }
        
        /* Message preparation */
        // Calculate shortest path with Dijkstra algorithm
        uint8_t dst_mesh_id = pkt_info.ble_data[DST_ADDR_IDX];
        uint8_t next_node_mesh_id = dijkstra_shortest_path(graph, MAX_MESH_SIZE,
                common_self_mesh_id, dst_mesh_id);
        if(next_node_mesh_id < 0) {
            printk("ERROR: Dijkstra algorithm failed.\n");
            return;
        }
        printk("Next hop: %d\n", next_node_mesh_id);
        
        // Header settings 
        pkt_info.ble_data[SENDER_ID_IDX] = common_self_mesh_id;
        pkt_info.ble_data[RCV_ADDR_IDX] = next_node_mesh_id;
        pkt_info.ble_data[TTL_IDX] = 0x01; // one jump allowed
        uint16_t timestamp = ble_add_packet_timestamp(pkt_info.ble_data);
        
        // Awaiting ack for this packet
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

        // Init adv params
        static struct bt_le_adv_param adv_tx_params = BT_LE_ADV_PARAM_INIT(
                BT_LE_ADV_OPT_USE_IDENTITY,
                BT_GAP_ADV_FAST_INT_MIN_1,
                BT_GAP_ADV_FAST_INT_MAX_1,
                NULL);
        
        // Lock the ble device
        k_mutex_lock(&ble_send_mutex, K_FOREVER);
        
        /* Sending a data message */
        err = bt_le_adv_start(&adv_tx_params, 
        ad_arr, ARRAY_SIZE(ad_arr), 
        NULL, 0);
        if(err) {
            printk("ERROR: could not start advertising : %d\n", err);
            return;
        }       
        printk("Started advertising a data packet.\n");

        // Wait for ack
        bool got_ack = ble_wait_for_ack(500);

        err = bt_le_adv_stop();
        if(err) {
            printk("ERROR: Failed to stop advertising %d.\n", err);
            return;
        }
        printk("Finished advertising a data packet.\n");
        
        // Release the device for ack thread to use 
        k_mutex_unlock(&ble_send_mutex);

        pkt_info.resend_counter++;
        printk("Resend counter: %d\n", pkt_info.resend_counter);
        
        // Remove flag indicating awaited ack
        err = k_msgq_get(&awaiting_ack, NULL, K_MSEC(50));
        
        // Distance recalculation and update 
        node_update_missed_transmissions(
                        &graph[pkt_info.ble_data[RCV_ADDR_IDX]], got_ack);
        uint8_t new_td = calc_td_from_missed_transmissions(
                    graph[pkt_info.ble_data[RCV_ADDR_IDX]].missed_transmissions);
        printk("New calculated TD: %d\n", new_td);
        graph_set_distance(graph,
                    common_self_mesh_id, pkt_info.ble_data[RCV_ADDR_IDX], new_td);

        // Put the message again into the queue and try to send it again
        if(!got_ack) {
            printk("##############################\n");
            printk("Did not receive ACK from node %d.\n", pkt_info.ble_data[RCV_ADDR_IDX]);
            printk("##############################\n");
            
            // Do not resend infinitely
            if(pkt_info.resend_counter < 3)k_msgq_put(
                    &data_packets_to_send_q, pkt_info.ble_data, K_NO_WAIT);
        }
    }
}


void ble_send_ack_thread_entry(
        void *unused1,
        void *unused2,
        void *unused3) {
    while(1){
        static ble_sender_info ack_info;
        int err = k_msgq_get(&ack_receivers_q, &ack_info, K_FOREVER);
        if(err){
            printk("ERROR: Reading from ack msg queue failed.\n");
            return;
        }
        
        // Prep ack packet
        static uint8_t ack_data[] = {
            0x05, 0x05, 0x05, 0x05, 0x05 ,0x05, 0x05, 0x05
        };

        ack_data[SENDER_ID_IDX] = common_self_mesh_id;
        ack_data[MSG_TYPE_IDX] = MSG_TYPE_ACK;
        ack_data[DST_ADDR_IDX] = 0xFF; //whatever, ignored
        ack_data[RCV_ADDR_IDX] = ack_info.node_id; // node to ack to
        ack_data[TIME_STAMP_MSB_IDX] = (0xFF00 & ack_info.time_stamp) >> 8;
        ack_data[TIME_STAMP_LSB_IDX] = 0x00FF & ack_info.time_stamp;
        ack_data[TTL_IDX] = 0x01; // One jump only is allowed
        
        static struct bt_data ad_arr[] = {
            BT_DATA(0xAA, ack_data, BLE_ACK_MSG_LEN) 
        };
        
        static struct bt_le_adv_param adv_tx_params = BT_LE_ADV_PARAM_INIT(
                BT_LE_ADV_OPT_USE_IDENTITY,
                BT_GAP_ADV_FAST_INT_MIN_1,
                BT_GAP_ADV_FAST_INT_MAX_1,
                NULL);
        
        // Advertise 
        // Lock the ble device
        k_mutex_lock(&ble_send_mutex, K_FOREVER);
        
        err = bt_le_adv_start(&adv_tx_params, 
        ad_arr, ARRAY_SIZE(ad_arr), 
        NULL, 0);
        if(err) {
            printk("ERROR: could not start advertising : %d\n", err);
            return;
        }
        
        k_msleep(200);
        
        err = bt_le_adv_stop();
        if(err) {
            printk("ERROR: Failed to stop advertising %d.\n", err);
            return;
        }
        
        // Unlock to allow data sending thread to use BLE
        k_mutex_unlock(&ble_send_mutex);

        printk("Finished advertising.\n");
    }
}


void ble_send_rt_thread_entry(struct node_t *graph) {
    while(true) {
        static uint8_t ble_data[BLE_RTR_MSG_LEN] = {0};
        static struct bt_data add_arr[] = {
            BT_DATA(0xAA, ble_data, BLE_RTR_MSG_LEN)
        };
        int err = k_msgq_get(&rtr_packets_to_send_q, &ble_data, K_FOREVER);
        if(err) {
            printk("ERROR: could not get packet from rtr_packets_to_send_q: %d\n", err);
            return;
        }
        
        // Header should not be touched as the message should be treated as
        // the same message among nodes

        static struct bt_le_adv_param adv_tx_params = BT_LE_ADV_PARAM_INIT(
                BT_LE_ADV_OPT_USE_IDENTITY,
                BT_GAP_ADV_FAST_INT_MIN_1,
                BT_GAP_ADV_FAST_INT_MAX_1,
                NULL);
        
        k_mutex_lock(&ble_send_mutex, K_FOREVER);
        printk("Sending rtr, source of it is: %d\n", ble_data[SENDER_ID_IDX]);
        err = bt_le_adv_start(&adv_tx_params, 
                add_arr, ARRAY_SIZE(add_arr),
                NULL, 0); 
        if(err) {
            printk("ERROR: could not start advertising routing table record %d.\n", err);
        }
        
        k_msleep(200);

        err = bt_le_adv_stop();
        if(err) {
            printk("ERROR: could not stop advertising routing table record %d.\n", err);
        }
        k_mutex_unlock(&ble_send_mutex);
    }
}


/* Callbacks */
void bt_msg_received_cb(const struct bt_le_scan_recv_info *info,
              struct net_buf_simple *buf){
    char addr_str[BT_ADDR_LE_STR_LEN];
    char data[BLE_LONGEST_MSG_LEN * 2 + 1];
    
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
        
    //printk("Received data from node with address: %s\n", addr_str);
    //printk("Data: %s\n", data);
    
    // Strip the buffer into simple byte array
    uint8_t ble_data[BLE_RTR_MSG_LEN] = {0}; 
    memcpy(ble_data, &(buf->data[2]), BLE_LONGEST_MSG_LEN);

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
            // Decrease a TTL counter for the packet for further processing
            //ble_data[TTL_IDX]--;

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
                        
                        uint8_t dst_mesh_id = ble_data[DST_ADDR_IDX];
                        if(dst_mesh_id == common_self_mesh_id) {
                            printk("FINAL DESTINATION REACHED\n");
                            // Do something with the data
                            return;
                        }
                        
                        // Queue message to process further
                        struct ble_packet_info pkt_info = {
                            .resend_counter = 0
                        };
                        memcpy(pkt_info.ble_data, ble_data, BLE_DATA_MSG_LEN);

                        err = k_msgq_put(
                                &data_packets_to_send_q, 
                                &pkt_info, K_NO_WAIT);
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
                            // This can happen when ack was received and flag was removed already, it's okay.
                            printk("ERROR: No info about awaited ack!\n");
                            printk("Data: %s\n", data);
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
                            k_wakeup(send_data_packet_thread_id);
                        }
                        break;
                    }
                    
                case MSG_TYPE_ROUTING_TAB:
                    // Send it further if time to live is not zero 
                    printk("RECEIVED RTR FROM %d\n", ble_data[SENDER_ID_IDX]);
                    load_rtr(graph, ble_data + HEADER_SIZE, BLE_RTR_MSG_LEN - HEADER_SIZE);
                    if(ble_data[TTL_IDX] > 1) {
                        printk("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
                        printk("Putting the other node rtr to send queue.\n");
                        printk("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
                        err = k_msgq_put(&rtr_packets_to_send_q, ble_data, K_NO_WAIT);
                        if(err) {
                            printk("ERROR: Could not put to RTR to send queue %d \n", err);
                        }
                    }
                    
                    print_graph(graph);
                    break;
            }
        }
    }
}


/* Utility functions */
void add_self_to_rtr_queue(struct k_timer *timer) {
    uint8_t buffer[BLE_RTR_MSG_LEN] = {0};
    
    // Initialize a header 
    buffer[SENDER_ID_IDX] = common_self_mesh_id;
    buffer[MSG_TYPE_IDX] = MSG_TYPE_ROUTING_TAB;
    buffer[DST_ADDR_IDX] = 0xFF;
    buffer[RCV_ADDR_IDX] = BROADCAST_ADDR;
    ble_add_packet_timestamp(buffer);
    buffer[TTL_IDX] = MAX_TTL;
    
    node_to_byte_array(graph + common_self_mesh_id, buffer + HEADER_SIZE, BLE_RTR_MSG_LEN - HEADER_SIZE);
    
    printk("Putting the self rtr to send queue.\n");
    int err = k_msgq_put(&rtr_packets_to_send_q, buffer, K_NO_WAIT);
    if(err) {
        printk("ERROR: Could not put the self RTR to send queue %d\n", err);
    }
}


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
                item->msg_type == ptr->msg_type) return true;        
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
    printk("Time remaining %d\n", time_remaining);
    return time_remaining > 0;
}
