#ifndef BLUETOOTH_BLE_H
#define BLUETOOTH_BLE_H
#include <sys/printk.h>
#include <bluetooth/hci.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/conn.h>
#include <bluetooth/rfcomm.h>
#include <bluetooth/sdp.h>
#include <bluetooth/iso.h>

// wrapper for few configuraion parameters for ble transsmission 
struct bt_config_t {
    // advertising 
    // ext_adv_options(uint32 my), bt_le_adv_params, set, 

    // scanning 
    struct bt_le_scan_param scan_params;
};

void bt_directed_messaging_setup(struct bt_config_t * config);
int bt_le_scan_start_wrapper(struct bt_config_t * config);

// callbacks 
static void bt_direct_msg_received_cb(const struct bt_le_scan_recv_info *info,
		      struct net_buf_simple *buf);


// some convinience wrapper for sending specific data to specific receiver 
// but with the same defined configuration 

#endif
