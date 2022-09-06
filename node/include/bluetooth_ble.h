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

#define BROADCAST_ADDR 0x7F	// 127

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
} __attribute__((aligned(4))) ble_sender_info;

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


/**
 * @brief Set BLE scan parameters which will be then used to start BLE scan. 
 * Register callback that will trigger on packet reception.
 *
 * @param scan_params - buffer to which params will be loaded.
 */
void ble_scan_setup(struct bt_le_scan_param *scan_params);


/**
 * @brief Consumer thread that will take data packets from 
 * data_packets_to_send_q and process them. This thread handles sending of data
 * packets and path calculations. Few major steps it performs:
 * - Prepares a message, sets the header and data fields.
 * - Sets the flag indicating on what ACK it is currently waiting.
 * - Locks with mutex BLE device and sends the packet.
 * - Then it will wait for the ACK.
 * - It will then recalculate transition cost.
 * - And if necessary wait until the neighbor finishes advertising so it will not be overwhelmed.
 *
 * @param graph - array with nodes.
 */
void ble_send_data_packet_thread_entry(struct node_t *graph);


/**
 * @brief Consumer thread that will take ACK packets from ack_receivers_q and 
 * process them. It will send the proper header, lock the BLE device and send the
 * ACK packet.
 */
void ble_send_ack_thread_entry();


/**
 * @brief Consumer thread with two main tasks: it will either send routing table
 * record (rtr) of self (self connections to other nodes and costs of those connections) 
 * or it will propagate received rtr from it's peers. Both of those are picked
 * from rtr_packets_to_send_q.
 *
 * @param graph - array with nodes.
 */
void ble_send_rtr_thread_entry(struct node_t *graph);

// callbacks 

/**
 * @brief Callback triggered on BLE packet reception. It keeps track of recently
 * received messages and ignores duplicates. It will convert the message to simple
 * byte array and then decide on what type of message it is and what routine 
 * should be triggered:
 * - MSG_TYPE_DATA
 * - MSG_TYPE_ERROR
 * - MSG_TYPE_ROUTING_TAB
 *
 * @param info - holds information about address of the sender.
 * @param buf - contains message data.
 */
void bt_msg_received_cb(const struct bt_le_scan_recv_info *info,
			struct net_buf_simple *buf);

/* Utility functions */

/**
 * @brief Check the received data packet if the node with given id is receiver of that
 * packet.
 *
 * @param data][] - buffer with data.
 * @param common_self_mesh_id - id of current node.
 *
 * @return true if node is receiver and false if it's not.
 */
bool ble_is_receiver(uint8_t data[], uint8_t id);


/**
 * @brief Stamp a data packet with current time stamp in correct fields of the 
 * header. 
 *
 * @param data][] - Provided data buffer, when function exits it will be stamped.
 *
 * @return - lower 32 bits of the current time stamp.
 */
uint16_t ble_add_packet_timestamp(uint8_t data[]);

/**
 * @brief Take the data buffer and extract from it time stamp.
 *
 * @param data][] - data buffer.
 *
 * @return - extracted timestamp.
 */
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

void rcv_pkts_cb_push(rcv_pkts_cb * cb, ble_sender_info * item);
void rcv_pkts_cb_pop(rcv_pkts_cb * cb);
bool rcv_pkts_cb_is_in_cb(rcv_pkts_cb * cb, ble_sender_info * item);

bool ble_wait_for_ack(int32_t timeout_ms);

void print_msgq_num_used(struct k_msgq *mq, char name[]);

#define VAR_NAME(mq) (#mq)
#endif
