#include <zephyr.h>
//#include <device.h>
//#include <devicetree.h>
//#include <drivers/gpio.h>
#include <sys/printk.h>
//#include <usb/usb_device.h>
#include <drivers/uart.h>
//#include <string.h>

#include "../include/graph.h"
#include "../include/dijkstra.h"

/* 1000 msec = 1 sec */
#define SLEEP_TIME_MS   1000

    
struct k_mutex graph_mutex;

void main(void)
{
    struct node_t * graph;
    uint8_t graph_init_error_code = graph_init(&graph, &graph_mutex);
    int counter = 0;

    // list test          
    //sys_slist_t lst;
    //uint8_t list_error_code = create_unvisited_slist(&lst, graph, 5, 0x0);
    //print_slist(&lst);
    //printk("list init code %d\n", list_error_code); 

    // dijkstra 
    dijkstra_shortest_path(graph, 5, 0, 2);
    
	while (1) {
        const struct device *dev;
        dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));
        //uart_poll_out(dev, 'A');
        
        //printk("Graph initialization status code: %d\n", error_code);
        //printk("Graph node: %d\n", graph->addr);
        //printk("Graph mutex lock count: %d \n", graph_mutex.lock_count);
        counter++;
		k_msleep(SLEEP_TIME_MS);
	}
}

