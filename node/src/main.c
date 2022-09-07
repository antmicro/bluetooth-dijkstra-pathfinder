#include <zephyr.h>
#include <sys/printk.h>
#include <drivers/uart.h>
#include <drivers/hwinfo.h>
#include <device.h>

#include "../include/graph.h"
#include "../include/dijkstra.h"
#include "../include/bluetooth_ble.h"

#define BLE_ADDR_LEN          18

// Rtr timings
#define RTR_TIMER_START_DELAY 2500
#define RTR_TIMER_PERIOD      60000

// Threads data
#define SEND_DATA_PACKET_THREAD_S_SIZE 1024
#define SEND_DATA_PACKET_THREAD_PRIO   2
K_THREAD_STACK_DEFINE(send_data_packet_thread_stack,
		      SEND_DATA_PACKET_THREAD_S_SIZE);

#define SEND_ACK_THREAD_S_SIZE 1024
#define SEND_ACK_THREAD_PRIO   1
K_THREAD_STACK_DEFINE(send_ack_thread_stack, SEND_ACK_THREAD_S_SIZE);

#define SEND_RTR_THREAD_S_SIZE 1024
#define SEND_RTR_THREAD_PRIO   6
K_THREAD_STACK_DEFINE(send_rtr_thread_stack, SEND_RTR_THREAD_S_SIZE);

K_THREAD_DEFINE(send_ack_thread, SEND_ACK_THREAD_S_SIZE,
		ble_send_ack_thread_entry, NULL, NULL, NULL,
		SEND_ACK_THREAD_PRIO, 0, 0);

// Timer to add current node to rtr propagation
K_TIMER_DEFINE(add_self_to_rtr_queue_timer, add_self_to_rtr_queue, NULL);

// Global variable so that other threads or IRQs can wake it up when necessary
k_tid_t send_data_packet_thread_id;

// Global data structure containing mesh information
struct node_t graph[MAX_MESH_SIZE];

void main(void)
{
	int err;
	// Graph Initialization
	err = graph_init(graph);
	__ASSERT(err == 0, "ERROR: Graph initialization failed (err %d)\n",
		 err);
	printk("Mesh contains %d nodes.\n", MAX_MESH_SIZE);

	// Bluetooth setup
	err = bt_enable(NULL);
	__ASSERT(err == 0, "ERROR: BLE initialization failed (err %d)\n", err);

	// Load common_self_ptr 
	char default_identity[BLE_ADDR_LEN] = { 0 };
	err = identify_self_in_graph(graph, default_identity, BLE_ADDR_LEN);
	__ASSERT(err == 0, "ERROR: Could not identify_self_in_graph (err %d)",
		 err);
	printk("Identified self with mesh id: %d and BLE address: %s",
	       common_self_ptr->addr, default_identity);

	struct bt_le_scan_param scan_params;
	ble_scan_setup(&scan_params);

	// Create Bluetooth LE threads
	struct k_thread send_data_packet_thread;
	send_data_packet_thread_id = k_thread_create(&send_data_packet_thread,
						     send_data_packet_thread_stack,
						     K_THREAD_STACK_SIZEOF
						     (send_data_packet_thread_stack),
						     ble_send_data_packet_thread_entry,
						     &graph, NULL, NULL,
						     SEND_DATA_PACKET_THREAD_PRIO,
						     0, K_NO_WAIT);
	k_thread_name_set(&send_data_packet_thread, "send_data_packet_thread");

	struct k_thread send_rtr_thread;
	k_thread_create(&send_rtr_thread,
			send_rtr_thread_stack,
			K_THREAD_STACK_SIZEOF
			(send_rtr_thread_stack),
			ble_send_rtr_thread_entry,
			&graph, NULL, NULL, SEND_RTR_THREAD_PRIO, 0, K_NO_WAIT);

	// Start counter that will add this node (self) to the routing table record
	// propagation thread to send to neighbors
	k_timer_start(&add_self_to_rtr_queue_timer,
		      K_MSEC(RTR_TIMER_START_DELAY), K_MSEC(RTR_TIMER_PERIOD));

	// Bluetooth scanning
	err = bt_le_scan_start(&scan_params, NULL);
	if (err) {
		printk("BLE scan error code: %d\n", err);
	}
}
