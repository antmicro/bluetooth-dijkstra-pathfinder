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

#define RCV_ADDR_IDX 2
#define DST_ADDR_IDX 3
#define BROADCAST_ADDR 0x7F // 127
#define ROUTING_TABLE_ID 0x7E //126

#define RECEIVEQ_PUT_TIMEOUT_MS 10

/* Events definitions */
#define BLE_SCANNED_EVENT                 0x1 

/* Global queues for BLE ad - hoc */
extern struct k_msgq common_received_packets_q;
extern struct k_fifo common_packets_to_send_q;

// TODO: make local mesh address into typedef for clarity
struct ble_tx_packet_data {
    void *fifo_reserved;
    uint8_t next_node_mesh_id; 
    struct net_buf_simple data;    
};

// temp data struct 
typedef struct {
    struct net_buf_simple buf;
}__attribute__((aligned(4))) ble_packet_buffer_alligned;


void ble_scan_setup(struct bt_le_scan_param *scan_params);
void ble_adv_sets_setup(struct node_t *graph, struct bt_le_ext_adv **adv_set);
void create_packet_thread_entry(struct node_t *graph);
void ble_send_packet_thread_entry(struct node_t *graph,
        struct bt_le_scan_param *params);
void get_mesh_id_from_data(struct net_buf_simple *buf, 
        uint8_t *mesh_id);

// callbacks 
void bt_msg_received_cb(
        const struct bt_le_scan_recv_info *info,
		      struct net_buf_simple *buf);

void ble_scanned(
        struct bt_le_ext_adv *adv,
        struct bt_le_ext_adv_scanned_info *info);

void ble_sent(struct bt_le_ext_adv *adv, 
        struct bt_le_ext_adv_sent_info *info);
#endif
