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

// Header indexes definitions
#define SENDER_ID_IDX 0
#define MSG_TYPE_IDX 1
#define DST_ADDR_IDX 2
#define RCV_ADDR_IDX 3
#define TIME_STAMP_MSB_IDX 4
#define TIME_STAMP_LSB_IDX 5
#define TTL_IDX 6
#define HEADER_SIZE 7

#define MSG_TYPE_DATA 0x1
#define MSG_TYPE_ROUTING_TAB 0x2
#define MSG_TYPE_ACK 0x3

#define BROADCAST_ADDR 0x7F // 127

#define BLE_DATA_MSG_LEN 8
#define BLE_RTR_MSG_LEN 24
#define BLE_ACK_MSG_LEN 8
#define BLE_LONGEST_MSG_LEN 24

#define RECEIVEQ_PUT_TIMEOUT_MS 10

#define CB_POP_TIME_MS 900

#define ACK_ADV_TIME_MS 100
#define DATA_ADV_TIME_MS 500

// Global variable with sender thread id to wake it up
extern k_tid_t send_data_packet_thread_id;

// Global variable for starting the add_self_to_rtr_queue_timer
extern struct k_timer add_self_to_rtr_queue_timer; 

typedef struct {
    uint8_t node_id;
    uint16_t time_stamp;
    uint8_t msg_type;
}__attribute__((aligned(4))) ble_sender_info;


struct ble_packet_info {
    uint8_t ble_data[BLE_DATA_MSG_LEN];
    uint8_t resend_counter;
};


typedef struct {
    ble_sender_info *buff_start;
    ble_sender_info *buff_end;
    size_t capacity;
    size_t count;
    ble_sender_info *head;
    ble_sender_info *tail;
} rcv_pkts_cb;

void ble_scan_setup(struct bt_le_scan_param *scan_params);
void ble_send_data_packet_thread_entry(struct node_t *graph);
void ble_send_ack_thread_entry();
void ble_send_rt_thread_entry(struct node_t *graph);


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
bool ble_is_receiver(uint8_t data[],uint8_t common_self_mesh_id);
uint16_t ble_add_packet_timestamp(uint8_t data[]);
uint16_t ble_get_packet_timestamp(uint8_t data[]);

// Timer callbacks
void add_self_to_rtr_queue(struct k_timer *timer);

// Circular buffer 
// Static initialization
#define RCV_PKTS_DEFINE(cb_name, cb_capacity) \
    ble_sender_info arr[cb_capacity]; \
    rcv_pkts_cb cb_name = {  \
        .buff_start = arr, \
        .buff_end = arr + cb_capacity,  \
        .capacity = cb_capacity, \
        .count = 0, \
        .head = arr, \
        .tail = arr\
    }
// head and tail set to arr_ptr are overkill but its done for the buff

void rcv_pkts_cb_push(rcv_pkts_cb *cb, ble_sender_info *item);
void rcv_pkts_cb_pop(rcv_pkts_cb *cb);
bool rcv_pkts_cb_is_in_cb(rcv_pkts_cb *cb, ble_sender_info *item);

bool ble_wait_for_ack(int32_t timeout_ms);

void print_msgq_num_used(struct k_msgq *mq, char name[]);

#define MSG_Q_NAME(mq) (#mq)
#endif









