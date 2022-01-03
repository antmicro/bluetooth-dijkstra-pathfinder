#ifndef DIJKSTRA_H
#define DIJKSTRA_H

#include "../include/graph.h"

uint8_t dijkstra_shortest_path(
        struct node_t * graph,
        uint8_t graph_size,
        uint8_t start_addr,
        uint8_t dst_addr);




#endif 
