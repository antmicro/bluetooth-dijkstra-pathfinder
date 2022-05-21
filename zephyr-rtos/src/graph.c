#include "../include/graph.h"
#include <bluetooth/bluetooth.h>

uint8_t common_self_mesh_id = 255;

void reset_td_visited(struct node_t *graph){
    for(uint8_t i = 0; i < MAX_MESH_SIZE; i++){
        if((graph + i)->reserved){
            (graph + i)->tentative_distance = INF;
            (graph + i)->visited = false;
        }
    }
}


void graph_update_distance(struct node_t *graph,
        uint8_t mesh_id_1, uint8_t mesh_id_2, uint8_t new_dist){
    struct node_t *node1 = &graph[mesh_id_1];
    struct node_t *node2 = &graph[mesh_id_2];
    // Do it both ways TODO: change it so the code is not copied
    for(uint8_t i = 0; i < node1->paths_size; i++){
        if(node1->paths[i].addr == mesh_id_2){
            node1->paths->distance = new_dist;
        }
    }
    for(uint8_t i = 0; i < node2->paths_size; i++){
        if(node2->paths[i].addr == mesh_id_1){
            node1->paths->distance = new_dist;
        }
    }
}


/* Utilities */
uint8_t identify_self_in_graph(struct node_t *graph){
    // get all configured identities 
    bt_addr_le_t identities[CONFIG_BT_ID_MAX];
    size_t *count = NULL;
    bt_id_get(NULL, count); // when addrs=null count will be loaded with num of ids 
    bt_id_get(identities, count);
    
    char addr_str[129]; // TODO: extract printks 
	bt_addr_le_to_str(&identities[0], addr_str, sizeof(addr_str));
    printk("Default identity: %s\n", addr_str);
    //printk("Value of DEVICEADDR[0] register: %u,\n", (NRF_FICR->DEVICEADDR[0]));

    uint8_t err = get_mesh_id_by_ble_addr(graph, addr_str, &common_self_mesh_id);
    if(err){
        printk("Error self identifying\n"); 
        return 1;
    }
    printk("Self identified in mesh as %d\n", common_self_mesh_id); 
    return 0;
}


// return 0 on succes, and > 0 on failure 
uint8_t get_mesh_id_by_ble_addr(struct node_t *graph, 
        char *ble_addr, uint8_t *mesh_id){
    // check in graph 
    for(uint8_t i = 0; i < MAX_MESH_SIZE; i++){
        if(!memcmp(graph[i].addr_bt_le, ble_addr, 17)){
            *mesh_id = graph[i].addr;
            return 0;
        }
    }
    return 1;
}
