#ifndef GRAPH_H 
#define GRAPH_H

#include <stdint.h>
#include <stdbool.h>
#include <kernel.h> // for k_k_malloc
#include <bluetooth/addr.h>

#define INF 255
#define MAX_MESH_SIZE 5 


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

    uint8_t paths_size;
    struct path_t * paths;
};


// global variable with address of this node 
extern uint8_t common_self_mesh_id; // TODO: maybe make it into abstract data type?

// global mutex for graph data structure
//extern struct k_mutex *graph_mutex;

//uint8_t graph_init(struct node_t *graph, struct k_mutex *graph_mutex);
uint8_t graph_init(struct node_t *graph);
void reset_tentative_distances(struct node_t *graph);
uint8_t identify_self_in_graph(struct node_t *graph);
uint8_t get_mesh_id_by_ble_addr(struct node_t *graph, char *ble_addr, uint8_t *mesh_id);

#endif
