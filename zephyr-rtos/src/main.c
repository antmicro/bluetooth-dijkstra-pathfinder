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
// data tx packets producer data 
#define CREATE_PACKET_THREAD_STACK_SIZE 1024
#define CREATE_PACKET_THREAD_PRIO       2 
K_THREAD_STACK_DEFINE(create_packet_thread_stack, 
        CREATE_PACKET_THREAD_STACK_SIZE);

// data transmitter thread data  
#define BLE_SEND_PACKET_THREAD_STACK_SIZE 1024
#define BLE_SEND_PACKET_THREAD_PRIO       2 
K_THREAD_STACK_DEFINE(ble_send_packet_thread_stack, 
        BLE_SEND_PACKET_THREAD_STACK_SIZE);

#define BLE_SEND_ACK_THREAD_STACK_SIZE 1024 
#define BLE_SEND_ACK_THREAD_PRIO       3
K_THREAD_DEFINE(send_ack_thred_id, BLE_SEND_ACK_THREAD_STACK_SIZE,
                ble_send_ack_thread_entry, NULL, NULL, NULL,
                BLE_SEND_ACK_THREAD_PRIO, 0, 0);

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
    struct k_thread create_packet_thread;
    k_tid_t create_packet_thread_id = k_thread_create(&create_packet_thread, 
            create_packet_thread_stack,
            K_THREAD_STACK_SIZEOF(create_packet_thread_stack),
            create_packet_thread_entry,
            &graph, NULL, NULL,
            CREATE_PACKET_THREAD_PRIO, 0, K_NO_WAIT);
    //printk("Created thread %d \n" ,create_packet_thread.stack_info.size);
    
     
    struct k_thread ble_send_packet_thread;
    k_tid_t ble_send_packet_thread_id = k_thread_create(&ble_send_packet_thread,
            ble_send_packet_thread_stack,
            K_THREAD_STACK_SIZEOF(ble_send_packet_thread_stack),
            ble_send_packet_thread_entry,
            &graph, &scan_params, NULL,
            BLE_SEND_PACKET_THREAD_PRIO, 0, K_NO_WAIT);
     
        
     
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

