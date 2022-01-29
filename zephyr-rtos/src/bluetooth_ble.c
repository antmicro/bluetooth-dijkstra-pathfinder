#include "../include/bluetooth_ble.h"
#include <zephyr.h>
#include <kernel/thread_stack.h>
#include "../include/dijkstra.h"


K_MSGQ_DEFINE(common_received_packets_q, sizeof(struct net_buf_simple), 10, 4);
K_FIFO_DEFINE(common_packets_to_send_q);

/* Setup functions */
void ble_scan_setup(struct bt_le_scan_param *scan_params){
    // directed messaging scan params
    *(scan_params) = (struct bt_le_scan_param){
		.type       = BT_LE_SCAN_TYPE_ACTIVE,
		.options    = BT_LE_SCAN_OPT_CODED, //+ BT_LE_SCAN_OPT_FILTER_DUPLICATE,
		.interval   = BT_GAP_SCAN_FAST_INTERVAL,
		.window     = BT_GAP_SCAN_FAST_WINDOW,
	};
    
    // register a callback for packet reception
    static struct bt_le_scan_cb scan_callbacks = {
        .recv = bt_direct_msg_received_cb,
    };
    bt_le_scan_cb_register(&scan_callbacks);
}


void ble_adv_sets_setup(struct node_t *graph, struct bt_le_ext_adv ***adv_set){ 
    uint32_t ext_adv_aptions = 
        BT_LE_ADV_OPT_EXT_ADV
        + BT_LE_ADV_OPT_DIR_MODE_LOW_DUTY
        + BT_LE_ADV_OPT_NOTIFY_SCAN_REQ;
  
    struct bt_le_adv_param params = {
        .id = 0x0,
        .options = ext_adv_aptions,
        .interval_min = 0xFF, // must be greater than 0x00a0
        .interval_max = 0x0300
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

        err = bt_le_ext_adv_create(&params, NULL, adv_set[i]);
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
    
    while(1){
        err = k_msgq_get(&common_received_packets_q, &buf, K_FOREVER);
        
        if(err){
            printk("Error reading from queue: %d \n!\n", err);
        }

        else{
            // retrieve dst node from data packet (3rd byte)
            get_mesh_id_from_data(&buf, &dst_mesh_id);
            printk("THIS IS DESTINATION MESH ADDR %d\n", dst_mesh_id);
            
            // calculate next node from dijkstra algorithm
            int next_node_mesh_id = dijkstra_shortest_path(graph, MAX_MESH_SIZE, 
                    common_self_mesh_id, dst_mesh_id);
            if(next_node_mesh_id < 0){
                printk("Dijkstra algorithm failed\n");
            }
            else{
                // if all is good, create a packet and queue it to tx 
                printk("Dijkstra result: %d\n", next_node_mesh_id);
                
                struct ble_tx_packet_data tx_packet = {
                    .next_node_mesh_id = next_node_mesh_id,
                    .data = buf
                };
                k_fifo_put(&common_packets_to_send_q, &tx_packet);
            }
        }
    }
}


void ble_send_packet_thread_entry(){
    struct ble_tx_packet_data *tx_packet;
    while(1){
        tx_packet = k_fifo_get(&common_packets_to_send_q, K_FOREVER);
        printk("This is next node's id from send thread %d \n", tx_packet->next_node_mesh_id);
    }
}


/* Callbacks */
static void bt_direct_msg_received_cb(const struct bt_le_scan_recv_info *info,
              struct net_buf_simple *buf){
    char addr_str[BT_ADDR_LE_STR_LEN];
    char data_str[31];
    
    // formatting 
    bin2hex(buf->data, buf->len, data_str, sizeof(data_str));
    bt_addr_le_to_str(info->addr, addr_str, sizeof(addr_str));

    // print
    printk("Received data from node with address: %s\n", addr_str);
    printk("Data: %s\n", data_str);
    
    // add to queue  
    int err = k_msgq_put(&common_received_packets_q, buf, K_NO_WAIT);
    if(err){ // TODO: change this purge, this is little extreme 
        printk("Error queue put: %d, queue purged\n", err);
        k_msgq_purge(&common_received_packets_q);
    }
}

