#ifndef GRAPH_H 
#define GRAPH_H

#include <stdint.h>
#include <stdbool.h>
#include <kernel.h> // for k_k_malloc

#define INF 255
#define MAX_MESH_SIZE 5 

struct path_t{
    uint8_t addr;
    uint8_t distance;
};

struct node_t{
    uint8_t addr;
    bool reserved; 

    bool visited;

    uint8_t tentative_distance;

    uint8_t paths_size;
    struct path_t * paths;
};

uint8_t graph_init(struct node_t ** graph, struct k_mutex * graph_mutex);
void reset_tentative_distances(struct node_t * graph);

#endif
