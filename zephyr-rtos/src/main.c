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
#define SEND_DATA_PACKET_THREAD_S_SIZE 1024
#define SEND_DATA_PACKET_THREAD_PRIO 3
K_THREAD_STACK_DEFINE(send_data_packet_thread_stack, 
        SEND_DATA_PACKET_THREAD_S_SIZE);

#define SEND_ACK_THREAD_S_SIZE 1024
#define SEND_ACK_THREAD_PRIO 1 
K_THREAD_STACK_DEFINE(send_ack_thread_stack, 
        SEND_ACK_THREAD_S_SIZE);

#define SEND_RT_THREAD_S_SIZE 1024
#define SEND_RT_THREAD_PRIO 2
K_THREAD_STACK_DEFINE(send_rt_thread_stack,
        SEND_RT_THREAD_S_SIZE);

K_THREAD_DEFINE(send_ack_thread, SEND_ACK_THREAD_S_SIZE,
ble_send_ack_thread_entry, NULL, NULL, NULL,
SEND_ACK_THREAD_PRIO, 0, 0);

k_tid_t send_data_packet_thread_id;
struct node_t graph[MAX_MESH_SIZE];

void main(void)
{
    /* Graph Initialization */
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
    struct k_thread send_data_packet_thread;
    send_data_packet_thread_id = k_thread_create(&send_data_packet_thread, 
            send_data_packet_thread_stack,
            K_THREAD_STACK_SIZEOF(send_data_packet_thread_stack),
            ble_send_data_packet_thread_entry,
            &graph, NULL, NULL,
            SEND_DATA_PACKET_THREAD_PRIO, 0, K_NO_WAIT);
    k_thread_name_set(&send_data_packet_thread, "send_data_packet_thread");

    struct k_thread send_rt_thread;
    k_tid_t send_rt_thread_id = k_thread_create(&send_rt_thread,
            send_rt_thread_stack,
            K_THREAD_STACK_SIZEOF(send_rt_thread_stack),
            ble_send_rt_thread_entry,
            &graph, NULL, NULL,
            SEND_RT_THREAD_PRIO, 0, K_NO_WAIT);
    
    // Start counter that will add self to the routing table record propagation thread
    k_timer_start(&add_self_to_rtr_queue_timer, K_MSEC(2000), K_MSEC(20000)); 

    
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

