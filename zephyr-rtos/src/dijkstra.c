
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
    create_unvisited_slist(&lst, graph, graph_size);

    // print the result 
    print_slist(&lst);

    // get smallest_td node and process it in a loop 
    uint8_t smallest_td_node_found;
    struct node_container * smallest_td_node_container;
    while(!(smallest_td_node_found =
                get_smallest_td_node(&lst, &smallest_td_node_container))){
        // check if not NULL
        if(!smallest_td_node_container){
            printk("Smallest td node container is NULL!\n");
            return 1;
        }
 
        // visit a smallest_td node and update its neighbours td
        recalculate_td_for_neighbours(smallest_td_node_container->node->addr, graph); 

        // check if smallest_td_node_container is destination node
        if(smallest_td_node_container->node->addr == dst_addr){
            printk("finish now\n\n\n\n");
            //free rest of the list
            free_slist(&lst);
            break;
        }

        // remove it from a list 
        remove_unvisited_slist_member(&lst, &smallest_td_node_container);
    }
    
    // trace back and record a path
    uint8_t paths_size;
    uint8_t * path;
    path = trace_back(graph, start_addr, dst_addr, &paths_size);
    printk("This is paths size: %d", paths_size);
    
    printk("Nodes in path:\n");
    for(uint8_t i = 0; i < paths_size; i++){
        printk("%d\n", *(path + i)); 
    }

    // unlock the mutex 
    int unlock_result = k_mutex_unlock(&graph_mutex);
    if(unlock_result){
        printk("Mutex unlock failed with status: %d\n", unlock_result); 
        return unlock_result;
    }

    return 0;
}


uint8_t get_smallest_td_node(sys_slist_t * lst, struct node_container ** container_buffer){
    // reset buffer
    *container_buffer = NULL;
    
    // setup 
    uint8_t smallest_td = INF;
    struct node_container * iterator;

    // check if empty
    if(sys_slist_is_empty(lst)){
        printk("List empty, returning\n");
        return 1; 
    }
    
    // find lowest_td_node
    SYS_SLIST_FOR_EACH_CONTAINER(lst, iterator, next_container_node_ptr){
        if(iterator->node->tentative_distance <= smallest_td){
            smallest_td = iterator->node->tentative_distance;
            *container_buffer = iterator;
            printk("found smallest td: %d\n", smallest_td);
        }
    }

    // if container_buffer is NULL then return error code
    if(!container_buffer) return 1;

    return 0;
}


void recalculate_td_for_neighbours(uint8_t node_addr, struct node_t * graph){
    struct node_t current_node = graph[node_addr]; 
    printk("1\n"); 
    // for each neighbour check distances
    for(uint8_t i = 0; i < current_node.paths_size; i++){
        // get neighbour
        struct node_t * neighbour = graph + (current_node.paths + i)->addr;
        printk("2\n");
        // consider only unvisited neighbours
        if(!neighbour->visited){
            uint8_t distance_to_neighbour = (current_node.paths + i)->distance;

            // check if distance through current node is smaller than neighbour's 
            // current tentative distance
            if(current_node.tentative_distance + distance_to_neighbour <
                    neighbour->tentative_distance){
                neighbour->tentative_distance = 
                    current_node.tentative_distance + distance_to_neighbour;
                neighbour->visited = true;
            }
        }
    }
}


uint8_t * trace_back(
        struct node_t * graph,
        uint8_t start_addr, 
        uint8_t dst_addr, 
        uint8_t * paths_len){
    // array to store longest path possible TODO: make it better than that
    uint8_t * path = k_malloc(sizeof(uint8_t) * MAX_MESH_SIZE);
    if(!path) return NULL;
 
    // tracing back
    uint8_t index = 0;
    struct node_t * current_node = graph + dst_addr;
    path[index] = current_node->addr;
    while(current_node->addr != start_addr){ 
        // check which of neighbours has smallest td 
        uint8_t smallest_td = INF;
        uint8_t number_of_paths = current_node->paths_size; 
        for(uint8_t i = 0; i < number_of_paths; i++){
            struct node_t * checked_node = graph + ((current_node->paths) + i)->addr;
            if(checked_node->tentative_distance <= smallest_td){
                smallest_td = checked_node->tentative_distance;
                current_node = checked_node;
            }
        }
        // smallest td node address add to a path 
        index++;
        path[index] = current_node->addr;
        printk("current node %d\n", current_node->addr);
    }

     
    *paths_len = index + 1;
    // invert path
    // two counters decrementing till they are equal and swapping contents
    return path;
}


uint8_t create_unvisited_slist(
        sys_slist_t * lst, 
        struct node_t * graph, 
        uint8_t graph_size){
    sys_slist_init(lst);
    for(uint8_t i = 0; i < graph_size; i++){
        //printk("iterating a node: %d\n", graph[i].addr); 
        if(graph[i].reserved){
            struct node_container * container = 
                k_malloc(sizeof(struct node_container));
            if(!container){
                printk("k_malloc failed in create_unvisited_list\n");
                return 1;
            }
            container->node = graph + i; 
            sys_slist_append(lst, &container->next_container_node_ptr); 
        }
    }
    return 0;
}


void free_slist(sys_slist_t * lst){
    volatile sys_snode_t * to_remove; 
    while((to_remove = sys_slist_get(lst))){
        struct node_container * container = CONTAINER_OF(
                to_remove, 
                struct node_container, 
                next_container_node_ptr);
        k_free(container);
    }
}


void remove_unvisited_slist_member(sys_slist_t * lst, struct node_container ** node_container_to_remove){
    // remove from a list 
    sys_slist_find_and_remove(lst,
            &(*node_container_to_remove)->next_container_node_ptr);
    //printk("three %d\n", rm);
    // free memory from heap 
    k_free(*node_container_to_remove);
    printk("four\n");
}


#ifdef DEBUG

void print_slist(sys_slist_t * lst){
    struct node_container * iterator;
    printk("Printing nodes in slist: \n");
    SYS_SLIST_FOR_EACH_CONTAINER(lst, iterator, next_container_node_ptr){
        printk("Node addr: %d\n", iterator->node->addr); 
    }
    printk("\n\n\n");
}

#endif 



















