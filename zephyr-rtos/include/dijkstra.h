#ifndef DIJKSTRA_H
#define DIJKSTRA_H

#include "../include/graph.h"

#define DEBUG 

struct node_container{
    sys_snode_t next_node;
    struct node_t * node;
};

uint8_t dijkstra_shortest_path(
        struct node_t * graph,
        uint8_t graph_size,
        uint8_t start_addr,
        uint8_t dst_addr);

uint8_t create_unvisited_list(
        sys_slist_t * lst, 
        struct node_t * graph, 
        uint8_t graph_size, 
        uint8_t start_addr);

#ifdef DEBUG 
void print_slist(sys_slist_t * lst);

#endif
#endif 
