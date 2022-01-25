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


// global mutex for graph data structure
extern struct k_mutex graph_mutex;
extern struct node_t graph[MAX_MESH_SIZE];


uint8_t graph_init();
void reset_tentative_distances();

#endif
