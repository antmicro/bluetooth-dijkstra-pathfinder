#include "../include/bluetooth_ble.h"

#define PASS_DATA_THREAD_STACK_SIZE 500
#define PASS_DATA_PRIORITY 5 

struct node_t graph[MAX_MESH_SIZE];

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


void bt_le_adv_sets_setup(struct bt_le_ext_adv ***adv_set){ 
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


//void pass_data(uint8_t dst, struct node_t * graph, uint8_t size){

//}


/* Callbacks */
static void bt_direct_msg_received_cb(const struct bt_le_scan_recv_info *info,
		      struct net_buf_simple *buf){
    char addr_str[BT_ADDR_LE_STR_LEN];
	bt_addr_le_to_str(info->addr, addr_str, sizeof(addr_str));
    printk("Received data from node with address: %s\n", addr_str);
    printk("Received data as trash: %d\n", buf->data[0]); 
}

