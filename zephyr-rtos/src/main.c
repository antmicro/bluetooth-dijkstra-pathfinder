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


void main(void)
{
    /* Graph Initialization */
    uint8_t graph_init_error_code = graph_init();
    if(graph_init_error_code){
        printk("Graph initialization failed! \n");
        return;
    }

    /* Dijkstra's path finding algorithm*/
    uint8_t dijkstra_err = 0;
    dijkstra_err = dijkstra_shortest_path(0, 2);
    if(dijkstra_err){
        printk("Dijkstra failed! \n");
        return;
    }

    int err = bt_enable(NULL);
    if(err){
        printk("BLE Initialization failed!\n");
    }

    /* Bluetooth scanning */
    struct bt_le_scan_param scan_params;
    bt_le_scan_setup(&scan_params);
    err  = bt_le_scan_start(&scan_params, NULL); 
    if(err){
        printk("BLE scan error code: %d\n", err);
    }

    /* Bluetooth direct adv setup*/
    static struct bt_le_ext_adv **adv_set[MAX_MESH_SIZE]; 
    //bt_le_adv_set_setup(adv_set);

    /* Debug */
    bt_addr_le_t my_identity[CONFIG_BT_ID_MAX];
    bt_id_get(my_identity, ARRAY_SIZE(my_identity));
    
    char hex[129];
    char hex2[129];
    bin2hex(my_identity->a.val, 6, hex2, 129);

	//bt_addr_le_to_str(my_identity, hex, sizeof(hex));
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

