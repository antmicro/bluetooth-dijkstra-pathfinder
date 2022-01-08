#include <stdint.h>
#include <stdlib.h>

#include "../include/graph.h"

uint8_t graph_init(struct node_t ** graph, struct k_mutex * graph_mutex){ 
    // graph mutex initialization 
    k_mutex_init(graph_mutex); 

    // nodes initialization 
    *graph = k_malloc(MAX_MESH_SIZE * sizeof(struct node_t));
    if((*graph) == NULL) return 1;
    
    // sink
    (*graph)->addr = 0x0;    
    (*graph)->reserved = true;
    (*graph)->visited = false;
    (*graph)->tentative_distance = INF;
    (*graph)->paths_size = 2;
    (*graph)->paths = k_malloc(sizeof(struct path_t) * (*graph)->paths_size);
    if((*graph)->paths == NULL) return 1;
    
    (*graph)->paths->addr = 0x1;
    (*graph)->paths->distance = 1;
    
    ((*graph)->paths + 1)->addr = 0x3;
    ((*graph)->paths + 1)->distance = 2;
   
    // node 0x1
    (*graph + 1)->addr = 0x1;    
    (*graph + 1)->reserved = true;
    (*graph + 1)->visited = false;
    (*graph + 1)->tentative_distance = INF;
    (*graph + 1)->paths_size = 2;
    (*graph + 1)->paths = k_malloc(sizeof(struct path_t) * (*graph)->paths_size);
    if((*graph + 1)->paths == NULL) return 1;
    
    (*graph + 1)->paths->addr = 0x0;
    (*graph + 1)->paths->distance = 1;
    
    ((*graph + 1)->paths + 1)->addr = 0x2;
    ((*graph + 1)->paths + 1)->distance = 1;
   
    // node 0x2
    (*graph + 2)->addr = 0x2;    
    (*graph + 2)->reserved = true;
    (*graph + 2)->visited = false;
    (*graph + 2)->tentative_distance = INF;
    (*graph + 2)->paths_size = 2;
    (*graph + 2)->paths = k_malloc(sizeof(struct path_t) * (*graph)->paths_size);
    if((*graph + 2)->paths == NULL) return 1;
    
    (*graph + 2)->paths->addr = 0x1;
    (*graph + 2)->paths->distance = 1;
    
    ((*graph + 2)->paths + 1)->addr = 0x3;
    ((*graph + 2)->paths + 1)->distance = 2;

    // node 0x4
    (*graph + 4)->addr = 0x4;    
    (*graph + 4)->reserved = true;
    (*graph + 4)->visited = false;
    (*graph + 4)->tentative_distance = INF;
    (*graph + 4)->paths_size = 2;
    (*graph + 4)->paths = k_malloc(sizeof(struct path_t) * (*graph)->paths_size);
    if((*graph + 4)->paths == NULL) return 1;
    
    (*graph + 4)->paths->addr = 0x2;
    (*graph + 4)->paths->distance = 2;
    
    ((*graph + 4)->paths + 1)->addr = 0x0;
    ((*graph + 4)->paths + 1)->distance = 2;
    
    // node 0x3 not initialized 
    (*graph + 3)->addr = 0x3;    
    (*graph + 3)->reserved = false;
    (*graph + 3)->visited = false;
    (*graph + 3)->tentative_distance = INF;
    (*graph + 3)->paths_size = 0;

    return 0;
}

void reset_tentative_distances(struct node_t * graph){
    for(uint8_t i = 0; i < MAX_MESH_SIZE; i++){
        if((graph + i)->reserved) (graph + i)->tentative_distance = INF;
    }
}
