#ifndef GRAPH_H 
#define GRAPH_H

#include <stdint.h>
#include <stdbool.h>
#include <kernel.h> // for k_k_malloc
#include <bluetooth/addr.h>

#define INF 255
//#define MAX_MESH_SIZE 9 


struct path_t{
    uint8_t addr;
    uint8_t distance;
};


struct node_t{
    uint8_t addr;
    char addr_bt_le[18];
    bool reserved; 

    bool visited;

    uint16_t tentative_distance;

    uint64_t missed_transmissions;

    uint8_t paths_size;
    struct path_t * paths;
};


// global variable with address of this node 
extern uint8_t common_self_mesh_id; 

// global mutex for graph data structure
//extern struct k_mutex *graph_mutex;

//uint8_t graph_init(struct node_t *graph, struct k_mutex *graph_mutex);
uint8_t graph_init(struct node_t *graph);
void reset_td_visited(struct node_t *graph);
void graph_set_distance(struct node_t *graph,
        uint8_t mesh_id_1, uint8_t mesh_id_2, uint8_t new_dist);
void node_update_missed_transmissions(struct node_t *node, 
        bool transmission_success);
uint8_t calc_td_from_missed_transmissions(uint64_t missed_transmissions);
uint8_t identify_self_in_graph(struct node_t *graph);
uint8_t get_mesh_id_by_ble_addr(struct node_t *graph, char *ble_addr, uint8_t *mesh_id);

#endif
