#include "../include/dijkstra.h"
#include "../include/graph_api_generated.h"
#include <string.h>


int dijkstra_shortest_path(
        struct node_t *graph,
        uint8_t graph_size,
        uint8_t start_addr, 
        uint8_t dst_addr){ 
    int err;
    // check if right node was picked 
    if(!graph[dst_addr].reserved || !graph[start_addr].reserved){
        printk("ERROR: Node address not used!\n");
        printk("ADDRS %d and %d\n", start_addr, dst_addr);
        return -1;
    }

    if(start_addr > MAX_MESH_SIZE || start_addr < 0
            || dst_addr > MAX_MESH_SIZE || dst_addr < 0){
        printk("Incorrect search address\n");
        return -1;
    }

    /* algorithm */
    // set tentative_distance to 0 at start node  
    err = node_t_tentative_distance_set(graph + start_addr, 0);
    if(err) {
        printk("ERROR: Could not set tentative distance\n");
        return -1;
    }
    
    // create unvisited list 
    sys_slist_t lst;
    create_unvisited_slist(graph, graph_size, &lst);

    // print the result 
    //print_slist(&lst);

    // get smallest_td node and process it in a loop 
    uint8_t smallest_td_node_found_code;
    struct node_container * smallest_td_node_container;
    while(!(smallest_td_node_found_code =
                get_smallest_td_node(&lst, &smallest_td_node_container))){
        if(!smallest_td_node_container){
            printk("Smallest td node container is NULL!\n");
            return -1;
        }
        
        // visit a smallest_td node and update its neighbours td
        recalculate_td_for_neighbours(smallest_td_node_container->node->addr, graph); 

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
    uint16_t * path;
    path = trace_back(graph, start_addr, dst_addr, &paths_size);
    
    // Path was not found -> fail
    if(!path) {
        printk("Path was not found\n");
        return -1;
    }
   
    printk("Path: \n");
    for(uint8_t i = 0; i < paths_size; i++){
        printk("%d\n", path[i]); 
    }
    
    // make all tentative_distances to INF again for future calculations
    reset_td_visited(graph); 

    // path is in reverse order, so next node from current one is
    // one before last one (last is start node) 
    int next_node_id = path[paths_size - 2];
    k_free(path);
    return next_node_id;
}


uint8_t get_smallest_td_node(
        sys_slist_t * lst, struct node_container ** container_buffer){
    // cleaer buffer
    *container_buffer = NULL;
     
    // setup 
    uint16_t smallest_td = INF;
    struct node_container * iterator;

    // check if empty
    if(sys_slist_is_empty(lst)){
        printk("List empty, returning\n");
        return 1; 
    }
    
    // find lowest_td_node
    SYS_SLIST_FOR_EACH_CONTAINER(lst, iterator, next_container_node_ptr){
        uint16_t td = 0;
        node_t_tentative_distance_get(iterator->node, &td);
        if(td <= smallest_td){
            smallest_td = td;
            *container_buffer = iterator;
        }
    }

    // if container_buffer is NULL then return error code
    if(!container_buffer) return 1;

    return 0;
}


void recalculate_td_for_neighbours(uint8_t node_addr, struct node_t *graph){
    struct node_t *current_node = graph + node_addr; 
    int err;
    for(uint8_t i = 0; i < current_node->paths_size; i++){ 
        struct node_t *neighbour = graph + ((current_node->paths + i)->addr);
         
        // consider only unvisited neighbours
        bool visited;
        node_t_visited_get(neighbour, &visited);
        if(!visited){
            struct path_t *path = current_node->paths + i;
            uint16_t cost_to_neighbour;
            path_t_cost_get(path, &cost_to_neighbour);

            // check if distance through current node is smaller than neighbour's 
            // current tentative distance
            uint16_t curr_node_td, neigh_td;
            node_t_tentative_distance_get(current_node, &curr_node_td); 
            node_t_tentative_distance_get(neighbour, &neigh_td);
            if(curr_node_td + cost_to_neighbour < neigh_td){
                err = node_t_tentative_distance_set(neighbour, 
                        curr_node_td + cost_to_neighbour);
                if(err) {
                    printk("ERROR: Could not set tentative_distance\n");
                }
            }
        }
    }
    err = node_t_visited_set(current_node, true);
    if(err) {
        printk("ERROR: Could not set visited\n");
    }
}


uint16_t * trace_back(
        struct node_t *graph,
        uint8_t start_addr, 
        uint8_t dst_addr, 
        uint8_t * paths_len){
    // array to store longest path possible TODO: make it better than that
    uint16_t * path = k_malloc(sizeof(uint16_t) * MAX_MESH_SIZE);
    if(!path) return NULL;
 
    // tracing back
    // setup
    uint8_t index = 0;
    struct node_t * current_node = graph + dst_addr;
    
    // get to node and check its neighbours for lowest td
    path[index] = current_node->addr;
    while(current_node->addr != start_addr){ 
        // check which of neighbours has smallest td 
        uint16_t smallest_td = INF;
        uint8_t number_of_paths = current_node->paths_size; 
        struct node_t *temp_node = current_node;
        for(uint8_t i = 0; i < number_of_paths; i++){
            struct node_t * checked_node = graph + ((current_node->paths) + i)->addr;
            uint16_t checked_node_td;
            node_t_tentative_distance_get(checked_node, &checked_node_td);
            if(checked_node_td <= smallest_td){
                smallest_td = checked_node_td;
                temp_node = checked_node; // here some temp node 
            }
        }
        
        // smallest td node address add to a path 
        current_node = temp_node;
        index++;
        path[index] = current_node->addr;
        
        // If path was not found...
        if(index > MAX_MESH_SIZE) {
            k_free(path); 

            *paths_len = 0; 
            return NULL;
        }
    }

    *paths_len = index + 1;
    return path;
}


uint8_t create_unvisited_slist(struct node_t *graph, 
        uint8_t graph_size, sys_slist_t * lst){
    sys_slist_init(lst);
    for(uint8_t i = 0; i < MAX_MESH_SIZE; i++){
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
}


#ifdef DEBUG

void print_slist(sys_slist_t * lst){
    struct node_container * iterator;
    printk("Printing nodes in slist: \n");
    SYS_SLIST_FOR_EACH_CONTAINER(lst, iterator, next_container_node_ptr){
        printk("Node addr: %d\n", iterator->node->addr); 
    }
    //printk("\n\n\n");
}

#endif 



















