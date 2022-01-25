#include "../include/bluetooth_ble.h"
#include <zephyr.h>
#include <kernel/thread_stack.h>

#define CREATE_PACKET_THREAD_STACK_SIZE 100
#define CREATE_PACKET_THREAD_PRIO       -2
K_THREAD_STACK_DEFINE(create_packet_thread_stack, 
        CREATE_PACKET_THREAD_STACK_SIZE);


void bt_le_scan_setup(struct bt_le_scan_param *scan_params){
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


void bt_le_adv_sets_setup(struct node_t *graph, struct bt_le_ext_adv ***adv_set){ 
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
    
    // get addr from string to proper format
    for(uint8_t i = 0; i < MAX_MESH_SIZE; i++){
        int err;
        bt_addr_t temp_ble_addr;
        err = bt_addr_from_str(graph[i].addr_bt_le, &temp_ble_addr);
        if(err){
            printk("Incorrect BLE addr string! err: %d\n", err);
            return;
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



void create_packet_thread_entry(struct net_buf_simple *buf){ 
    printk("Inside entry\n");
}


/* Callbacks */
static void bt_direct_msg_received_cb(const struct bt_le_scan_recv_info *info,
		      struct net_buf_simple *buf){
    char addr_str[BT_ADDR_LE_STR_LEN];
    char data_str[31];
   
    printk("juz nie moge\n\n");

    // formatting 
    bin2hex(buf->data, buf->len, data_str, sizeof(data_str));
	bt_addr_le_to_str(info->addr, addr_str, sizeof(addr_str));

    // print
    printk("Received data from node with address: %s\n", addr_str);
    printk("Data: %s\n", data_str);
    struct k_thread create_packet_thread;
    k_tid_t my_tid = k_thread_create(&create_packet_thread, 
            create_packet_thread_stack,
            K_THREAD_STACK_SIZEOF(create_packet_thread_stack),
            create_packet_thread_entry,
            buf, NULL, NULL,
            CREATE_PACKET_THREAD_PRIO, 0, K_NO_WAIT);
    printk("Created thread %d \n" ,create_packet_thread.stack_info.size);
}

