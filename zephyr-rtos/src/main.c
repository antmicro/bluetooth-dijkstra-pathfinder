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
#define CREATE_PACKET_THREAD_STACK_SIZE 100
#define CREATE_PACKET_THREAD_PRIO       1 
K_THREAD_STACK_DEFINE(create_packet_thread_stack, 
        CREATE_PACKET_THREAD_STACK_SIZE);


// queue of packets to pass 
K_FIFO_DEFINE(packets_to_send);

void main(void)
{
    // TODO: move important variables to the top for better visibility
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
    bt_le_scan_setup(&scan_params);
   
    /* Create Bluetooth LE threads */
    struct k_thread create_packet_thread;
    k_tid_t create_packet_thread_id = k_thread_create(&create_packet_thread, 
            create_packet_thread_stack,
            K_THREAD_STACK_SIZEOF(create_packet_thread_stack),
            create_packet_thread_entry,
            &graph, NULL, NULL,
            CREATE_PACKET_THREAD_PRIO, 0, K_NO_WAIT);
    printk("Created thread %d \n" ,create_packet_thread.stack_info.size);
    

        
    /* Dijkstra's path finding algorithm*/
    //uint8_t dijkstra_err = 0;
    //dijkstra_err = dijkstra_shortest_path(graph, MAX_MESH_SIZE, 0, 2);
    //if(dijkstra_err){
    //    printk("Dijkstra failed! \n");
    //    return;
    //}

    /* Bluetooth scanning */
    err  = bt_le_scan_start(&scan_params, NULL); 
    if(err){
        printk("BLE scan error code: %d\n", err);
    }

    /* Bluetooth direct adv setup*/
    //static struct bt_le_ext_adv **adv_set[MAX_MESH_SIZE]; 
    //bt_le_adv_set_setup(adv_set);
    
    /* Debug */
    while (1) {
        const struct device *dev;
        dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));
        printk("here");
        //printk("Graph initialization status code: %d\n", graph_init_error_code);
        //printk("Graph mutex lock count: %d \n", graph_mutex.lock_count);
		k_msleep(SLEEP_TIME_MS);
	}
}

