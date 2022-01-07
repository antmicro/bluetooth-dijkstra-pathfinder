
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
    // TODO: make checking if correct addresses were passed  
    // algorithm
    // set tentative_distance to 0 at start node  
    graph[start_addr].tentative_distance = 0;  // not a pointer ? no cuz [] is dereference
    
    // create unvisited list 
    sys_slist_t lst;
    create_unvisited_slist(&lst, graph, graph_size, start_addr);
 
    // print the result 
    print_slist(&lst);

    // get smallest_td node and process it in a loop 
    uint8_t smallest_td_node_found;
    struct node_container smallest_td_node_container;
    while(!(smallest_td_node_found = get_smallest_td_node(&lst, &smallest_td_node_container))){
        // check if smallest_td_node_container is destination node
        if(smallest_td_node_container.node->addr == dst_addr){
            //free rest of the list
            free_slist(&lst);
            break;
        }

        // visit a smallest_td node and update its neighbours td
        recalculate_td_for_neighbours(smallest_td_node_container.node->addr, graph); 
        
        // remove it from a list 
        remove_unvisited_slist_member(&lst, &smallest_td_node_container);
    }
    
    // trace back and record a path

    // unlock the mutex 
    int unlock_result = k_mutex_unlock(&graph_mutex);
    if(unlock_result){
        printk("Mutex unlock failed with status: %d\n", unlock_result); 
        return unlock_result;
    }

    return 0;
}


uint8_t get_smallest_td_node(sys_slist_t * lst, struct node_container * container_buffer){
    uint8_t smallest_td = 0;
    struct node_container * iterator;
    if(sys_slist_is_empty(lst)) return 1; 
    SYS_SLIST_FOR_EACH_CONTAINER(lst, iterator, next_container_node_ptr){
        if(iterator->node->tentative_distance < smallest_td){
            smallest_td = iterator->node->tentative_distance;
            container_buffer = iterator;
        }
    }
    return 0;
}


void recalculate_td_for_neighbours(uint8_t node_addr, struct node_t * graph){
    struct node_t current_node = graph[node_addr]; 
    
    // for each neighbour check distances
    for(uint8_t i = 0; i < current_node.paths_size; i++){
        // get neighbour
        struct node_t neighbour = graph[(current_node.paths + i)->addr];
        
        // consider only non visited neighbours
        if(!neighbour.visited){
            uint8_t distance_to_neighbour = (current_node.paths + i)->distance;

            // check if distance through current node is smaller than neighbour's 
            // current tentative distance
            if(current_node.tentative_distance + distance_to_neighbour <
                    neighbour.tentative_distance){
                neighbour.tentative_distance = 
                    current_node.tentative_distance + distance_to_neighbour;
                neighbour.visited = true;
            }
        }
    }
}


uint8_t create_unvisited_slist(
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
            sys_slist_append(lst, &container->next_container_node_ptr); 
        }
    }
    return 0;
}


void free_slist(sys_slist_t * lst){
    struct node_container * iterator;
    SYS_SLIST_FOR_EACH_CONTAINER(lst, iterator, next_container_node_ptr){
        remove_unvisited_slist_member(lst, iterator);   
    }
}


void remove_unvisited_slist_member(sys_slist_t * lst, struct node_container * node_to_remove){
    // remove from a list 
    sys_slist_find_and_remove(lst, &node_to_remove->next_container_node_ptr);

    // free memory from heap 
    k_free(node_to_remove);
}


#ifdef DEBUG

void print_slist(sys_slist_t * lst){
    struct node_container * iterator;
    printk("Printing nodes in slist: \n");
    SYS_SLIST_FOR_EACH_CONTAINER(lst, iterator, next_container_node_ptr){
        printk("Node addr: %d\n", iterator->node->addr); 
    }
}

#endif 



















