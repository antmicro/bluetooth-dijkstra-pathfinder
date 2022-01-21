
#include "../include/dijkstra.h"
#include <string.h>

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
    
    if(start_addr > MAX_MESH_SIZE || start_addr < 0
            || dst_addr > MAX_MESH_SIZE || dst_addr < 0){
        printk("Incorrect search address\n");
        return 1;
    }

    /* algorithm */
    // set tentative_distance to 0 at start node  
    graph[start_addr].tentative_distance = 0;
    
    // create unvisited list 
    sys_slist_t lst;
    create_unvisited_slist(&lst, graph, graph_size);

    // print the result 
    print_slist(&lst);

    // get smallest_td node and process it in a loop 
    uint8_t smallest_td_node_found_code;
    struct node_container * smallest_td_node_container;
    while(!(smallest_td_node_found_code =
                get_smallest_td_node(&lst, &smallest_td_node_container))){
        if(!smallest_td_node_container){
            printk("Smallest td node container is NULL!\n");
            return 1;
        }
 
        // visit a smallest_td node and update its neighbours td
        recalculate_td_for_neighbours(smallest_td_node_container->node->addr, graph); 
        printk("address found: %d\n", smallest_td_node_container->node->addr);
        // check if smallest_td_node_container is destination node
        if(smallest_td_node_container->node->addr == dst_addr){
            //free rest of the list
            free_slist(&lst);
            break;
        }

        // remove processed node from a list 
        remove_unvisited_slist_member(&lst, &smallest_td_node_container);
    }
    
    // trace back and record a path
    uint8_t paths_size;
    uint8_t * path;
    path = trace_back(graph, start_addr, dst_addr, &paths_size);
   
    // print nodes in path
    printk("Nodes in path:\n");
    char output[40] = "Path: ";
    for(uint8_t i = 0; i < paths_size; i++){
        char buffer[4];
        snprintk(buffer, 4, "%d, ", *(path + i));
        strcat(output, buffer);
    }
    printk("%s\n", output);
    printk("pathssize %d\n", paths_size);
    printk("%d, %d, %d", *(path), *(path+1), *(path+2));
    

    // unlock the mutex 
    int unlock_result = k_mutex_unlock(&graph_mutex);
    if(unlock_result){
        printk("Mutex unlock failed with status: %d\n", unlock_result); 
        return unlock_result;
    }

    return 0;
}


uint8_t get_smallest_td_node(
        sys_slist_t * lst, struct node_container ** container_buffer){
    // cleaer buffer
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
        }
    }

    // if container_buffer is NULL then return error code
    if(!container_buffer) return 1;

    return 0;
}


void recalculate_td_for_neighbours(uint8_t node_addr, struct node_t * graph){
    struct node_t current_node = graph[node_addr]; 
    printk("Current node: %d\n", current_node.addr); 
    printk("Current node td: %d\n", current_node.tentative_distance);
    // for each neighbour check distances
    printk("Current node paths_size: %d\n", current_node.paths_size);
    for(uint8_t i = 0; i < current_node.paths_size; i++){
        // get neighbour
        printk("Neighbour address %d\n", ((current_node.paths + i)->addr));
        struct node_t * neighbour = graph + ((current_node.paths + i)->addr);
        printk("    Neighbour: %d\n", neighbour->addr);
         
        // consider only unvisited neighbours
        if(!neighbour->visited){
            uint8_t distance_to_neighbour = (current_node.paths + i)->distance;
            printk("    Distance to neighbour: %d\n", distance_to_neighbour);

            // check if distance through current node is smaller than neighbour's 
            // current tentative distance
            if(current_node.tentative_distance + distance_to_neighbour <
                    neighbour->tentative_distance){
                printk("     node %d Pre update: %d\n",
                        neighbour->addr, neighbour->tentative_distance);
                neighbour->tentative_distance = 
                    current_node.tentative_distance + distance_to_neighbour;
                printk("     node after update %d\n", neighbour->tentative_distance);
            }
        }
    }
    (graph + node_addr)->visited = true;
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
    // setup
    uint8_t index = 0;
    struct node_t * current_node = graph + dst_addr;
    
    // get to node and check its neighbours for lowest td
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
    // TODO:invert path
    // two counters decrementing till they are equal and swapping contents
    return path;
}


uint8_t create_unvisited_slist(
        sys_slist_t * lst, 
        struct node_t * graph, 
        uint8_t graph_size){
    sys_slist_init(lst);
    for(uint8_t i = 0; i < graph_size; i++){
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
    sys_slist_find_and_remove(lst,
            &(*node_container_to_remove)->next_container_node_ptr);
    
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



















