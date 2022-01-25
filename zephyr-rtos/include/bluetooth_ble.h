#ifndef BLUETOOTH_BLE_H
#define BLUETOOTH_BLE_H
#include <sys/printk.h>
#include <bluetooth/hci.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/conn.h>
#include <bluetooth/rfcomm.h>
#include <bluetooth/sdp.h>
#include <bluetooth/iso.h>
#include "graph.h"


void bt_le_scan_setup(struct bt_le_scan_param *scan_params);
void bt_le_adv_set_setup(struct bt_le_ext_adv ***adv_set);

// callbacks 
static void bt_direct_msg_received_cb(const struct bt_le_scan_recv_info *info,
		      struct net_buf_simple *buf);


// some convinience wrapper for sending specific data to specific receiver 
// but with the same defined configuration 

#endif
