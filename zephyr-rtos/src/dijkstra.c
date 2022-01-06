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
        return lock_result;
    }
    
    // algorithm
    // set tentative_distance to 0 at start node  
    graph[start_addr].tentative_distance = 0;  // not a pointer ? no cuz [] is dereference
     
    // create unvisited list 
    

    // unlock the mutex 
    int unlock_result = k_mutex_unlock(&graph_mutex);
    if(unlock_result){
        printk("Mutex unlock failed with status: %d\n", unlock_result); 
        return unlock_result;
    }

    return 0;
}

uint8_t create_unvisited_list(
        sys_slist_t * lst, 
        struct node_t * graph, 
        uint8_t graph_size, 
        uint8_t start_addr){
    sys_slist_init(lst);
    for(uint8_t i = 0; i < graph_size; i++){
        printk("iterating a node: %d\n", graph[i].addr); 
        if(graph[i].reserved && graph[i].addr != start_addr){
            // TODO: free containers !!! does remove func free containers?
            // remove func acts on sys_node_t so I have to free container after removing from list 
            struct node_container * container = k_malloc(sizeof(struct node_container));
            if(!container){
                printk("k_malloc failed in create_unvisited_list\n");
                return 1;
            }
            container->node = graph + i; //prepend or append faster? all constant in time 
            sys_slist_append(lst, &container->next_node); 
        }
    }
    return 0;
}

#ifdef DEBUG

void print_slist(sys_slist_t * lst){
    struct node_container * iterator;
    printk("Printing nodes in slist: \n");
    SYS_SLIST_FOR_EACH_CONTAINER(lst, iterator, next_node){
        printk("Node addr: %d\n", iterator->node->addr); 
    }

}

#endif 



















