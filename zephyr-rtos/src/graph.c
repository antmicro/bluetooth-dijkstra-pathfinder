#include "../include/graph.h"
#include <bluetooth/bluetooth.h>
#include <math.h>

uint8_t common_self_mesh_id = 255;

void reset_td_visited(struct node_t graph[]){
    for(uint8_t i = 0; i < MAX_MESH_SIZE; i++){
        if((graph + i)->reserved){
            (graph + i)->tentative_distance = INF;
            (graph + i)->visited = false;
        }
    }
}


void graph_set_distance(struct node_t graph[],
        uint8_t mesh_id_1, uint8_t mesh_id_2, uint8_t new_dist){
    struct node_t *node1 = &graph[mesh_id_1];
    struct node_t *node2 = &graph[mesh_id_2];
    // Do it both ways TODO: change it so the code is not copied
    for(uint8_t i = 0; i < node1->paths_size; i++){
        if(node1->paths[i].addr == mesh_id_2){
            node1->paths[i].distance = new_dist;
        }
    }
    for(uint8_t i = 0; i < node2->paths_size; i++){
        if(node2->paths[i].addr == mesh_id_1){
            node2->paths[i].distance = new_dist;
        }
    }
}


// TODO: Should have some decay time, so the communication is retried
void node_update_missed_transmissions(struct node_t *node, 
        bool transmission_success){
    if(transmission_success && node->missed_transmissions > 0){
        node->missed_transmissions--;
    }
    else node->missed_transmissions++;
}


uint8_t calc_td_from_missed_transmissions(uint64_t missed_transmissions){
    float a = 0.5; 
    float y = a * missed_transmissions * missed_transmissions + 1.0;
    return (uint8_t)y;
}


/* Routing table propagation */
void node_to_byte_array(struct node_t *node, uint8_t buffer[], uint8_t buffer_size) {
    // First two fields in buffer are always the same and identify the node
    // with relation to which the rest of connections is made
    buffer[0] = node->addr;
    buffer[1] = node->paths_size;

    // Rest of fields represent the connections of that node
    for(uint8_t i = 0; i < node->paths_size; i++) {
        uint8_t neighbor_addr_idx = 2 * i + 2 + 0;
        uint8_t neighbor_dist_idx = 2 * i + 2 + 1;
        buffer[neighbor_addr_idx] = (node->paths + i)->addr;
        buffer[neighbor_dist_idx] = (node->paths + i)->distance;
    }
}


size_t node_get_size_in_bytes(struct node_t *node) {
    // Check if the node is reserved
    if(!node->reserved) return 0;
    
    // Include only those fields that will be sent in routing table 
    size_t byte_size = sizeof(node->addr) + sizeof(node->paths_size) 
        * (sizeof(node->paths->addr) + sizeof(node->paths->distance));
    return byte_size;
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
