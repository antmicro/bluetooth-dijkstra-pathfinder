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

#define MSG_TYPE_IDX 2
#define DST_ADDR_IDX 3
#define RCV_ADDR_IDX 4
#define TIME_STAMP_UPPER_IDX 5
#define TIME_STAMP_LOWER_IDX 6

#define MSG_TYPE_DATA 0x1
#define MSG_TYPE_ROUTING_TAB 0x2
#define MSG_TYPE_ACK 0x3

#define BROADCAST_ADDR 0x7F // 127

#define RECEIVEQ_PUT_TIMEOUT_MS 10

/* Events definitions */
#define BLE_SCANNED_EVENT                 0x1 

/* Global queues for BLE ad - hoc */
extern struct k_msgq common_received_packets_q;
extern struct k_fifo common_packets_to_send_q;

// TODO: make local mesh address into typedef for clarity
struct ble_tx_packet_data {
    void *fifo_reserved;
    struct net_buf_simple data;    
};

// temp data struct 
typedef struct {
    struct net_buf_simple buf;
}__attribute__((aligned(4))) ble_packet_buffer_alligned;

struct ble_ack_info {
    uint8_t node_id;
    uint16_t time_stamp;
};

void ble_scan_setup(struct bt_le_scan_param *scan_params);
void create_packet_thread_entry(struct node_t *graph);
void ble_send_packet_thread_entry(struct node_t *graph,
        struct bt_le_scan_param *params);

// callbacks 
void bt_msg_received_cb(
        const struct bt_le_scan_recv_info *info,
		      struct net_buf_simple *buf);

void ble_scanned(
        struct bt_le_ext_adv *adv,
        struct bt_le_ext_adv_scanned_info *info);

void ble_sent(struct bt_le_ext_adv *adv, 
        struct bt_le_ext_adv_sent_info *info);

/* Utility functions */
bool ble_is_receiver(struct net_buf_simple *buf,uint8_t common_self_mesh_id);
void ble_add_packet_timestamp(uint8_t data[]);
#endif
