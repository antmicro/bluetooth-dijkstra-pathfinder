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

#define RECEIVEQ_PUT_TIMEOUT_MS 10

/* Global queues for BLE ad - hoc */
extern struct k_msgq common_received_packets_q;

// TODO: make local mesh address into typedef for clarity
struct bt_le_packet_data {
    uint8_t local_mesh_addr; 
    struct net_buf_simple data;    
};

// temp data struct that 
typedef struct {
    struct net_buf_simple buf;
}__attribute__((aligned(4))) ble_packet_buffer_alligned;


void bt_le_scan_setup(struct bt_le_scan_param *scan_params);
void bt_le_adv_set_setup(struct node_t *graph, struct bt_le_ext_adv ***adv_set);
void create_packet_thread_entry(struct node_t *graph);
void get_ble_dst_addr_from_data(struct net_buf_simple *buf, 
        uint8_t *ble_addr);

// callbacks 
static void bt_direct_msg_received_cb(const struct bt_le_scan_recv_info *info,
		      struct net_buf_simple *buf);


#endif
