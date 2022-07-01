#include <zephyr.h>
//#include <device.h>
//#include <devicetree.h>
//#include <drivers/gpio.h>
#include <sys/printk.h>
//#include <usb/usb_device.h>
#include <drivers/uart.h>
//#include <string.h>
#include <drivers/hwinfo.h>
#include <device.h>

#include "../include/graph.h"
#include "../include/dijkstra.h"
#include "../include/bluetooth_ble.h"

/* 1000 msec = 1 sec */
#define SLEEP_TIME_MS   1000

/* Threads data */
#define PREP_DATA_PACKET_THREAD_S_SIZE 1024
#define PREP_DATA_PACKET_THREAD_PRIO 2 
K_THREAD_STACK_DEFINE(prep_data_packet_thread_stack, 
        PREP_DATA_PACKET_THREAD_S_SIZE);

#define PREP_ACK_THREAD_S_SIZE 1024
#define PREP_ACK_THREAD_PRIO 2 
K_THREAD_STACK_DEFINE(prep_ack_thread_stack, 
        PREP_ACK_THREAD_S_SIZE);

#define SEND_PACKET_THREAD_S_SIZE 1024 
#define SEND_PACKET_THREAD_PRIO 1 
K_THREAD_STACK_DEFINE(send_packet_thread_stack, 
        SEND_PACKET_THREAD_S_SIZE);

K_THREAD_DEFINE(prep_ack_thread, PREP_ACK_THREAD_S_SIZE,
ble_prep_ack_thread_entry, NULL, NULL, NULL,
PREP_ACK_THREAD_PRIO, 0, 0);

void main(void)
{
    /* Graph Initialization */
    struct node_t graph[MAX_MESH_SIZE];
    uint8_t graph_init_error_code = graph_init(graph);
    if(graph_init_error_code){
        printk("Graph initialization failed! \n");
        return;
    }
    
    /* Bluetooth setup */ 
    int err = bt_enable(NULL);
    if(err){
        printk("BLE Initialization failed!\n");
    }

    err = identify_self_in_graph(graph);
    if(err){
        printk("BLE self identification failed!\n");
    }
    
    struct bt_le_scan_param scan_params;
    ble_scan_setup(&scan_params);
    

    printk("BUILT FOR %d NUMBER OF NODES\n", MAX_MESH_SIZE);

    /* Create Bluetooth LE threads */
    struct k_thread prep_data_packet_thread;
    k_tid_t prep_data_packet_thread_id = k_thread_create(&prep_data_packet_thread, 
            prep_data_packet_thread_stack,
            K_THREAD_STACK_SIZEOF(prep_data_packet_thread_stack),
            ble_prep_data_packet_thread_entry,
            &graph, NULL, NULL,
            PREP_DATA_PACKET_THREAD_PRIO, 0, K_NO_WAIT);
    /* 
    struct k_thread prep_ack_thread;
    k_tid_t prep_ack_thread_id = k_thread_create(&prep_ack_thread,
            prep_ack_thread_stack,
            K_THREAD_STACK_SIZEOF(prep_ack_thread_stack),
            ble_prep_ack_thread_entry,
            NULL, NULL, NULL,
            PREP_ACK_THREAD_PRIO, 0, K_NO_WAIT);
    */

    struct k_thread send_packet_thread;
    k_tid_t send_packet_thread_id = k_thread_create(&send_packet_thread,
            send_packet_thread_stack,
            K_THREAD_STACK_SIZEOF(send_packet_thread_stack),
            ble_send_packet_thread_entry,
            &graph, &scan_params, NULL,
            SEND_PACKET_THREAD_PRIO, 0, K_NO_WAIT);
     
    /* Bluetooth scanning */
    err  = bt_le_scan_start(&scan_params, NULL); 
    if(err){
        printk("BLE scan error code: %d\n", err);
    }

    /* Debug */
    while (1) {
        const struct device *dev;
        dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));

        //printk("Graph initialization status code: %d\n", graph_init_error_code);
        //printk("Graph mutex lock count: %d \n", graph_mutex.lock_count);
		k_msleep(SLEEP_TIME_MS);
	}
}

