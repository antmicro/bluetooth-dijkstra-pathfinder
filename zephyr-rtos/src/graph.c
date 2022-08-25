#include "../include/graph.h"
#include "../include/graph_api_generated.h"
#include "kernel.h"
#include <bluetooth/bluetooth.h>
#include <math.h>

uint8_t common_self_mesh_id = 255;

void reset_td_visited(struct node_t graph[]){
    for(uint8_t i = 0; i < MAX_MESH_SIZE; i++){
        if((graph + i)->reserved){
            node_t_tentative_distance_set(graph + i, INF);
            node_t_visited_set(graph + i, false);
        }
    }
}

// Setters and getters respecting the mutex access
int node_t_visited_set(struct node_t *node, bool new_val) {
    int err;
    err = k_mutex_lock(&node->node_mutex, K_FOREVER);
    if(err) return err;
    node->visited = new_val;
    k_mutex_unlock(&node->node_mutex);
    if(err) return err;
    return 0;
}

int node_t_visited_get(struct node_t *node, bool *ret_val) {
    int err;
    err = k_mutex_lock(&node->node_mutex, K_FOREVER);
    if(err) return err;
    *ret_val = node->visited;
    k_mutex_unlock(&node->node_mutex);
    if(err) return err;
    return 0;
}

int node_t_tentative_distance_set(struct node_t *node, uint16_t new_val) {
    int err;
    err = k_mutex_lock(&node->node_mutex, K_FOREVER);
    if(err) return err;
    node->tentative_distance = new_val;
    k_mutex_unlock(&node->node_mutex);
    if(err) return err;
    return 0;
}

int node_t_tentative_distance_get(struct node_t *node, uint16_t *ret_val) {
    int err;
    err = k_mutex_lock(&node->node_mutex, K_FOREVER);
    if(err) return err;
    *ret_val = node->tentative_distance;
    k_mutex_unlock(&node->node_mutex);
    if(err) return err;
    return 0;
}

int path_t_cost_set(struct path_t *path, uint16_t new_val){
    int err;
    err = k_mutex_lock(&path->path_mutex, K_FOREVER);
    if(err) return err;
    path->cost = new_val;
    err = k_mutex_unlock(&path->path_mutex);
    if(err) return err;
    return 0;
}

int path_t_cost_get(struct path_t *path, uint16_t *ret_val) {
    int err;
    err = k_mutex_lock(&path->path_mutex, K_FOREVER);
    if(err) return err;
    *ret_val = path->cost;
    err = k_mutex_unlock(&path->path_mutex);
    if(err) return err;
    return 0;
}

void graph_set_cost(struct node_t graph[],
        uint8_t mesh_id_1, uint8_t mesh_id_2, uint8_t new_cost){
    struct node_t *node1 = graph + mesh_id_1;
    struct node_t *node2 = graph + mesh_id_2;
    // Do it both ways 
    for(uint8_t i = 0; i < node1->paths_size; i++){
        if(node1->paths->addr == mesh_id_2){
            path_t_cost_set(node1->paths + i, new_cost);
        }
    }
    for(uint8_t i = 0; i < node2->paths_size; i++){
        if(node2->paths->addr == mesh_id_1){
            path_t_cost_set(node2->paths + i, new_cost);
        }
    }
}


// TODO: Should have some decay time, so the communication is retried
/*
void node_update_missed_transmissions(struct node_t *node, 
        bool transmission_success){
    if(transmission_success && node->missed_transmissions > 0){
        node->missed_transmissions--;
    }
    else node->missed_transmissions++;
}
*/


uint16_t calc_distance_from_missed_transmissions(uint64_t missed_transmissions){
    float a = 0.5; 
    float y = a * missed_transmissions * missed_transmissions + 1.0;
    return (uint16_t)y;
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
        uint16_t cost;
        path_t_cost_get(node->paths + i, &cost);
        buffer[neighbor_addr_idx] = (node->paths + i)->addr;
        buffer[neighbor_dist_idx] = cost;
    }
}


size_t node_get_size_in_bytes(struct node_t *node) {
    // Check if the node is reserved
    if(!node->reserved) return 0;
    
    // Include only those fields that will be sent in routing table 
    size_t byte_size = sizeof(node->addr) + sizeof(node->paths_size) 
        * (sizeof(node->paths->addr) + sizeof(node->paths->cost));
    return byte_size;
}


void load_rtr(struct node_t graph[], uint8_t buff[], uint8_t size) {
    // Each node is encoded as: 
    // addr | number of peers | peer1 addr | peer1 dist | peer2 addr | peer2 addr
    
    // First two bytes of each rtr are self addr and n neighs
    uint8_t node_addr = buff[0];
    uint8_t neighs_n = buff[1];

    // Then iterate over node neighs 
    for(uint8_t j = 0; j < neighs_n; j++) {
        uint8_t idx = 1 + 1 + (2 * j);
        if(idx > size) {
            printk("ERROR: index out of bounds when loading routing table.\n");
            return;
        }
        uint8_t neigh_addr = buff[idx];
        uint8_t neigh_dist = buff[idx + 1];
        load_node_info(&graph[node_addr], neigh_addr, neigh_dist); 
    }
}


void print_graph(struct node_t graph[]) {
    printk("Mesh topology:\n");
    uint16_t cost;
    for(uint8_t i = 0; i < MAX_MESH_SIZE; i++) {
        printk("Node %d\n", graph[i].addr);
        printk("Connected to: \n");
        for(uint8_t j = 0; j < graph[i].paths_size; j++) {
            path_t_cost_get(graph[i].paths + j, &cost);
            printk("    Node %d with distance: %d \n",
                    graph[i].paths[j].addr, cost);
        }
    }
}


void load_node_info(struct node_t *node, uint8_t neigh_addr, uint8_t cost) {
    for(uint8_t i = 0; i < node->paths_size; i++) {
        if(node->paths[i].addr == neigh_addr) {
            path_t_cost_set(node->paths + i, cost);
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
