
/* THIS FILE IS AUTO GENERATED - DO NOT MODIFY DIRECTLY */
#include <stdint.h>
#include <stdlib.h>
#include <bluetooth/addr.h>

#include "../../include/graph.h"

//uint8_t graph_init(struct node_t *graph, struct k_mutex *graph_mutex){ 
uint8_t graph_init(struct node_t *graph){ 
   // graph mutex initialization 
    //k_mutex_init(graph_mutex); 

    // node 0x0 
    graph[0].addr = 0x0;
    strncpy(graph[0].addr_bt_le, "C0:00:00:00:00:00", 18);
    graph[0].reserved = true;
    graph[0].visited = false;
    graph[0].tentative_distance = INF;
    graph[0].paths_size = 3;
    
    graph[0].paths = k_malloc(sizeof(struct path_t) * graph[0].paths_size);
    if(graph[0].paths == NULL) return 1;
    
    (graph[0].paths + 0)->addr = 0x4;
    (graph[0].paths + 0)->distance = 1;
    
    (graph[0].paths + 1)->addr = 0x2;
    (graph[0].paths + 1)->distance = 7;
    
    (graph[0].paths + 2)->addr = 0x5;
    (graph[0].paths + 2)->distance = 6;
    
    
    // node 0x1 
    graph[1].addr = 0x1;
    strncpy(graph[1].addr_bt_le, "C0:00:00:00:00:01", 18);
    graph[1].reserved = true;
    graph[1].visited = false;
    graph[1].tentative_distance = INF;
    graph[1].paths_size = 2;
    
    graph[1].paths = k_malloc(sizeof(struct path_t) * graph[1].paths_size);
    if(graph[1].paths == NULL) return 1;
    
    (graph[1].paths + 0)->addr = 0x6;
    (graph[1].paths + 0)->distance = 8;
    
    (graph[1].paths + 1)->addr = 0x4;
    (graph[1].paths + 1)->distance = 5;
    
    
    // node 0x2 
    graph[2].addr = 0x2;
    strncpy(graph[2].addr_bt_le, "C0:00:00:00:00:02", 18);
    graph[2].reserved = true;
    graph[2].visited = false;
    graph[2].tentative_distance = INF;
    graph[2].paths_size = 2;
    
    graph[2].paths = k_malloc(sizeof(struct path_t) * graph[2].paths_size);
    if(graph[2].paths == NULL) return 1;
    
    (graph[2].paths + 0)->addr = 0x0;
    (graph[2].paths + 0)->distance = 7;
    
    (graph[2].paths + 1)->addr = 0x3;
    (graph[2].paths + 1)->distance = 6;
    
    
    // node 0x3 
    graph[3].addr = 0x3;
    strncpy(graph[3].addr_bt_le, "C0:00:00:00:00:03", 18);
    graph[3].reserved = true;
    graph[3].visited = false;
    graph[3].tentative_distance = INF;
    graph[3].paths_size = 2;
    
    graph[3].paths = k_malloc(sizeof(struct path_t) * graph[3].paths_size);
    if(graph[3].paths == NULL) return 1;
    
    (graph[3].paths + 0)->addr = 0x5;
    (graph[3].paths + 0)->distance = 5;
    
    (graph[3].paths + 1)->addr = 0x2;
    (graph[3].paths + 1)->distance = 4;
    
    
    // node 0x4 
    graph[4].addr = 0x4;
    strncpy(graph[4].addr_bt_le, "C0:00:00:00:00:04", 18);
    graph[4].reserved = true;
    graph[4].visited = false;
    graph[4].tentative_distance = INF;
    graph[4].paths_size = 3;
    
    graph[4].paths = k_malloc(sizeof(struct path_t) * graph[4].paths_size);
    if(graph[4].paths == NULL) return 1;
    
    (graph[4].paths + 0)->addr = 0x0;
    (graph[4].paths + 0)->distance = 4;
    
    (graph[4].paths + 1)->addr = 0x1;
    (graph[4].paths + 1)->distance = 9;
    
    (graph[4].paths + 2)->addr = 0x5;
    (graph[4].paths + 2)->distance = 8;
    
    
    // node 0x5 
    graph[5].addr = 0x5;
    strncpy(graph[5].addr_bt_le, "C0:00:00:00:00:05", 18);
    graph[5].reserved = true;
    graph[5].visited = false;
    graph[5].tentative_distance = INF;
    graph[5].paths_size = 4;
    
    graph[5].paths = k_malloc(sizeof(struct path_t) * graph[5].paths_size);
    if(graph[5].paths == NULL) return 1;
    
    (graph[5].paths + 0)->addr = 0x6;
    (graph[5].paths + 0)->distance = 6;
    
    (graph[5].paths + 1)->addr = 0x0;
    (graph[5].paths + 1)->distance = 8;
    
    (graph[5].paths + 2)->addr = 0x3;
    (graph[5].paths + 2)->distance = 8;
    
    (graph[5].paths + 3)->addr = 0x4;
    (graph[5].paths + 3)->distance = 9;
    
    
    // node 0x6 
    graph[6].addr = 0x6;
    strncpy(graph[6].addr_bt_le, "C0:00:00:00:00:06", 18);
    graph[6].reserved = true;
    graph[6].visited = false;
    graph[6].tentative_distance = INF;
    graph[6].paths_size = 2;
    
    graph[6].paths = k_malloc(sizeof(struct path_t) * graph[6].paths_size);
    if(graph[6].paths == NULL) return 1;
    
    (graph[6].paths + 0)->addr = 0x5;
    (graph[6].paths + 0)->distance = 7;
    
    (graph[6].paths + 1)->addr = 0x1;
    (graph[6].paths + 1)->distance = 8;
    
    
    
return 0;
}