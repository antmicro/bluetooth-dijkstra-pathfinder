#include "../include/dijkstra.h"


uint8_t dijkstra_shortest_path(
        struct node_t * graph,
        uint8_t graph_size,
        uint8_t start_addr,
        uint8_t dst_addr){
    // lock the mutex
    extern struct k_mutex graph_mutex;
    int lock_result = k_mutex_lock(&graph_mutex, K_FOREVER);
    if(lock_result){
        printk("Mutex lock failed with status: %d\n", lock_result); 
        return 1;
    }
    
    // algorithm
    // set tentative_distance to 0 at start node  
    graph[start_addr].tentative_distance = 0;  // not a pointer ????
    
    // create unvisited list 


    // unlock the mutex 
    int unlock_result = k_mutex_unlock(&graph_mutex);
    if(unlock_result){
        printk("Mutex unlock failed with status: %d\n", unlock_result); 
    }

    return 1;
}

