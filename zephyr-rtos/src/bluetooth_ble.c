#include "../include/bluetooth_ble.h"

void bt_directed_messaging_setup(struct bt_config_t * config){
    
    // directed messaging scan params
    config->scan_params = (struct bt_le_scan_param){
		.type       = BT_LE_SCAN_TYPE_ACTIVE,
		.options    = BT_LE_SCAN_OPT_CODED,
		.interval   = BT_GAP_SCAN_FAST_INTERVAL,
		.window     = BT_GAP_SCAN_FAST_WINDOW,
	};
    
    // register a callback for packet reception
    static struct bt_le_scan_cb scan_callbacks = {
        .recv = bt_direct_msg_received_cb,
    };
    bt_le_scan_cb_register(&scan_callbacks);
}


int bt_le_scan_start_wrapper(struct bt_config_t * config){
    return bt_le_scan_start(&config->scan_params, NULL);
}


static void bt_direct_msg_received_cb(const struct bt_le_scan_recv_info *info,
		      struct net_buf_simple *buf){
    char addr_str[BT_ADDR_LE_STR_LEN];
	bt_addr_le_to_str(info->addr, addr_str, sizeof(addr_str));
    printk("Received data from node with address: %s\n", addr_str);
}

