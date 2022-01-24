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

void dijkstra(){
    // TODO: make dijkstra to a thread 
    // TODO: make BLE to thread with mutex 
}

// global mutex for graph data structure
struct k_mutex graph_mutex;


static void device_found(const bt_addr_le_t *addr, int8_t rssi, uint8_t type,
			 struct net_buf_simple *ad)
{
	char addr_str[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(addr, addr_str, sizeof(addr_str));
	printk("Device found: %s (RSSI )\n", addr_str);

    char data_str[129];
    bin2hex(ad->data, ad->len, data_str, sizeof(data_str));
    printk("Data as hex:%s\n", data_str);
}


void main(void)
{
    /* Graph Initialization */
    struct node_t graph[MAX_MESH_SIZE];
    uint8_t graph_init_error_code = graph_init(graph, &graph_mutex);
    if(graph_init_error_code){
        printk("Graph initialization failed! \n");
        return;
    }

    /* Dijkstra's path finding algorithm*/
    uint8_t dijkstra_err = 0;
    dijkstra_err = dijkstra_shortest_path(graph, 5, 0, 2);
    if(dijkstra_err){
        printk("Dijkstra failed! \n");
        return;
    }

    /* Bluetooth advertising */
    struct bt_config_t bt_config;
    bt_directed_messaging_setup(&bt_config); 
    
    int err = bt_enable(NULL);
    if(err){
        printk("BLE Initialization failed!\n");
    }

    err  = bt_le_scan_start_wrapper(&bt_config);
    printk("BLE scan error code: %d\n", err);
    
    bt_addr_le_t my_identity[CONFIG_BT_ID_MAX];
    bt_id_get(my_identity, ARRAY_SIZE(my_identity));
    
    char hex[129];
    char hex2[129];
    bin2hex(my_identity->a.val, 6, hex2, 129);

	bt_addr_le_to_str(my_identity, hex, sizeof(hex));
    printk("a.val: %s\n", hex); 
    printk("my_identity: %s\n", hex);

    printk("NRF_FICR->DEVICEADDR[0] %u,\n", (NRF_FICR->DEVICEADDR[0]));
    while (1) {
        const struct device *dev;
        dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));
        //printk("Graph initialization status code: %d\n", graph_init_error_code);
        //printk("Graph mutex lock count: %d \n", graph_mutex.lock_count);
		k_msleep(SLEEP_TIME_MS);
	}
}

