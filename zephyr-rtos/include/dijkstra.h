#ifndef DIJKSTRA_H
#define DIJKSTRA_H

#include "../include/graph.h"

struct sys_slist_t a;
sys_slist_init(&a);

uint8_t dijkstra_shortest_path(
        struct node_t * graph,
        uint8_t graph_size,
        uint8_t start_addr,
        uint8_t dst_addr);




#endif 
