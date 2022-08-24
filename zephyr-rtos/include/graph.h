#ifndef GRAPH_H 
#define GRAPH_H

#include <stdint.h>
#include <stdbool.h>
#include <kernel.h> // for k_k_malloc
#include <bluetooth/addr.h>

#define INF 65535


struct path_t{
    uint8_t addr;
    uint16_t distance;
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
extern struct node_t graph[MAX_MESH_SIZE];

uint8_t graph_init(struct node_t *graph);
void reset_td_visited(struct node_t *graph);

void graph_set_distance(struct node_t *graph,
        uint8_t mesh_id_1, uint8_t mesh_id_2, uint8_t new_dist);
void node_update_missed_transmissions(struct node_t *node, 
        bool transmission_success);
uint16_t calc_distance_from_missed_transmissions(uint64_t missed_transmissions);

void node_to_byte_array(struct node_t *node, uint8_t buffer[], uint8_t buffer_size);
size_t node_get_size_in_bytes(struct node_t *node);
void load_rtr(struct node_t graph[], uint8_t buff[], uint8_t size);
void load_node_info(struct node_t *node, uint8_t neigh_addr, uint8_t dist);
void print_graph(struct node_t graph[]);

uint8_t identify_self_in_graph(struct node_t *graph);
uint8_t get_mesh_id_by_ble_addr(struct node_t *graph, char *ble_addr, uint8_t *mesh_id);

#endif
