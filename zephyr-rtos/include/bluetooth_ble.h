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

#define SENDER_ID_IDX 1
#define MSG_TYPE_IDX 2
#define DST_ADDR_IDX 3
#define RCV_ADDR_IDX 4
#define TIME_STAMP_UPPER_IDX 5
#define TIME_STAMP_LOWER_IDX 6

#define MSG_TYPE_DATA 0x1
#define MSG_TYPE_ROUTING_TAB 0x2
#define MSG_TYPE_ACK 0x3

#define BROADCAST_ADDR 0x7F // 127

#define BLE_MSG_LEN 8

#define RECEIVEQ_PUT_TIMEOUT_MS 10

#define CB_POP_TIME_MS 900

/* Events definitions */
#define BLE_ACK_RECEIVED_EVENT 0x1 

/* Global queues for BLE ad - hoc */
//extern struct k_msgq common_received_packets_q;
//extern struct k_fifo common_packets_to_send_q;

// TODO: make local mesh address into typedef for clarity
struct ble_tx_packet_data {
    void *fifo_reserved;
    struct net_buf_simple data;    
};

typedef struct {
    uint8_t node_id;
    uint16_t time_stamp;
    uint8_t msg_type;
}__attribute__((aligned(4))) ble_sender_info;

typedef struct {
    ble_sender_info *buff_start;
    ble_sender_info *buff_end;
    size_t capacity;
    size_t count;
    ble_sender_info *head;
    ble_sender_info *tail;
} rcv_pkts_cb;

void ble_scan_setup(struct bt_le_scan_param *scan_params);
void ble_prep_data_packet_thread_entry(struct node_t *graph);
void ble_prep_ack_thread_entry();
void ble_send_packet_thread_entry(struct node_t *graph,
        struct bt_le_scan_param *params);
void ble_send_ack_thread_entry();


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

uint16_t ble_add_packet_timestamp(uint8_t data[]);
uint16_t ble_get_packet_timestamp(uint8_t data[]);

// Circular buffer 
// Static initialization
#define RCV_PKTS_DEFINE(cb_name, cb_capacity) \
    static ble_sender_info arr[cb_capacity]; \
    static rcv_pkts_cb cb_name = {  \
        .buff_start = arr, \
        .buff_end = arr + cb_capacity * sizeof(ble_sender_info), \
        .capacity = cb_capacity, \
        .count = 0, \
        .head = arr, \
        .tail = arr \
    }

void rcv_pkts_cb_push(rcv_pkts_cb *cb, ble_sender_info *item);
void rcv_pkts_cb_pop(rcv_pkts_cb *cb);
bool rcv_pkts_cb_is_in_cb(rcv_pkts_cb *cb, ble_sender_info *item);
#endif









