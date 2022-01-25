#include "../include/graph.h"
#include <bluetooth/bluetooth.h>

void reset_tentative_distances(struct node_t *graph){
    for(uint8_t i = 0; i < MAX_MESH_SIZE; i++){
        if((graph + i)->reserved) (graph + i)->tentative_distance = INF;
    }
}

uint8_t identify_self_in_graph(struct node_t *graph){
    uint8_t common_self_id;
    // get all configured identities 
    bt_addr_le_t identities[CONFIG_BT_ID_MAX];
    size_t *count = NULL;
    bt_id_get(NULL, count); // when addrs=null count will be loaded with num of ids 
    bt_id_get(identities, count);
    
    char addr_str[129];
	bt_addr_le_to_str(&identities[0], addr_str, sizeof(addr_str));
    printk("Default identity: %s\n", addr_str);
    printk("Value of DEVICEADDR[0] register: %u,\n", (NRF_FICR->DEVICEADDR[0]));

    for(uint8_t i = 0; i < MAX_MESH_SIZE; i++){
        if(!strcmp(graph[i].addr_bt_le, addr_str)){
            common_self_id = graph[i].addr; // grrrrr
            printk("Self identified in mesh as %d\n", common_self_id); 
            return 0;
        }
    }
                
    printk("Error during self identification, no member in graph with %s BLE address\n",
        addr_str);
    return 1;
}
