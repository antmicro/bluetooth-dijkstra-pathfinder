#ifndef GRAPH_H 
#define GRAPH_H

#include <stdint.h>
#include <stdbool.h>
#include <kernel.h> 
#include <bluetooth/addr.h>
#include "graph_api_generated.h"

#define INF 65535 // PZIE: If you're aiming for a "max number" then at least go with hex


struct node_t{
    struct k_mutex node_mutex;
    uint8_t addr;
    char addr_bt_le[18];
    bool reserved; 

    bool visited;

    uint16_t tentative_distance;

    uint8_t paths_size;
    struct path_t * paths;
};

// PZIE: I'd say this is a nice place to put comments in
extern uint8_t common_self_mesh_id; 
extern struct node_t graph[MAX_MESH_SIZE];

uint8_t graph_init(struct node_t *graph);

// Dijkstra's utility 
void reset_td_visited(struct node_t *graph);

// Setters and getters respecting the mutex access
int node_t_visited_set(struct node_t *node, bool new_val); 
int node_t_visited_get(struct node_t *node, bool *ret_val); 
int node_t_tentative_distance_set(struct node_t *node, uint16_t new_val);
int node_t_tentative_distance_get(struct node_t *node, uint16_t *ret_val);
int path_t_cost_set(struct path_t *path, uint16_t new_val);
int path_t_cost_get(struct path_t *path, uint16_t *ret_val);

void graph_set_cost(struct node_t *graph,
        uint8_t mesh_id_1, uint8_t mesh_id_2, uint8_t new_cost);
void node_update_missed_transmissions(struct node_t *node, 
        bool transmission_success);
uint16_t calc_distance_from_missed_transmissions(uint64_t missed_transmissions);

void node_to_byte_array(struct node_t *node, uint8_t buffer[], uint8_t buffer_size);
size_t node_get_size_in_bytes(struct node_t *node);
void load_rtr(struct node_t graph[], uint8_t buff[], uint8_t size);
void load_node_info(struct node_t *node, uint8_t neigh_addr, uint8_t cost);
void print_graph(struct node_t graph[]);

uint8_t identify_self_in_graph(struct node_t *graph);
uint8_t get_mesh_id_by_ble_addr(struct node_t *graph, char *ble_addr, uint8_t *mesh_id);

#endif
