#ifndef DIJKSTRA_H
#define DIJKSTRA_H

#include "../include/graph.h"

#define DEBUG 

struct node_container{
    sys_snode_t next_container_node_ptr;
    struct node_t * node;
};

// global variable updated with path after dijkstra algorithm  
extern uint8_t * path; // TODO: make init function

/**
 * @brief Perform shortest path algorithm on graph, between start and dst adrresses.
 * Function locks graph mutex, reads and modifies graph members. 
 * TODO: in docs sum up all things that this  function modifies according to other funcs
 * TODO: make other functions static 
 * TODO: add path inverting function
 * TODO: add path returning
 * TODO: add graph resetting function, called at the beginning of di 
 * @param start_addr address of starting node. 
 * @param dst_addr address of destination node.
 *
 * @return 0 on succes > 0 on failure.
 */
uint8_t dijkstra_shortest_path(
        struct node_t *graph,
        uint8_t graph_size,
        uint8_t start_addr,
        uint8_t dst_addr);

/**
 * @brief Get the slist container with node that has the smallest tentative_distance 
 * for algorithm purposes. If function fails buffer will be NULL and it will return
 * 1.
 *
 * @param lst pointer to a unvisited list, from which to pick node with smallest td.
 * @param container_buffer buffer with slist container that stores node with smallest td. 
 * if list is empty, so be careful dereferencing it.
 *
 * @return 0 if node was found and 1 if list is empty.
 */
uint8_t get_smallest_td_node(sys_slist_t * lst, struct node_container ** container_buffer);


// get to a node, iterate thorugh its neighbours, check if path thorugh current node is smaller
// than current tentative_distance of a neighbour


/**
 * @brief Checks a node's neighbours if their tentative_distance is smaller through
 * current node, if so it is modified and node is marked as visited.
 *
 * @param node_addr
 */
void recalculate_td_for_neighbours(uint8_t node_addr, struct node_t *graph); 

// need initialized memory heap
uint8_t * trace_back(
        struct node_t *graph,
        uint8_t start_addr, 
        uint8_t dst_addr, 
        uint8_t * paths_len);


/**
 * @brief Create a list of nodes for search, excluding starting node. List is single
 * linked list data structure, as it will be oftnely iterated through and after
 * each iteration one of elements (with lowest td) will be processed and removed.
 * Members of a list - node_containers are malloced and caller is responsible for 
 * freeing!
 *
 * @param lst Pointer to list.
 * @param start_addr address of a node that is starting node.
 *
 * @return 
 */
uint8_t create_unvisited_slist(struct node_t *graph, 
        uint8_t graph_size, sys_slist_t * lst);


/**
 * @brief Free unvisited list from remaining members.
 *
 * @param lst unvisited list.
 */
void free_slist(sys_slist_t * lst);


/**
 * @brief Removes a member from a list and frees a memory k_malloced for node container.
 *
 * @param lst list to remove a member.
 * @param node_to_remove member to remove.
 */
void remove_unvisited_slist_member(sys_slist_t * lst, struct node_container ** node_to_remove);

#ifdef DEBUG 

/**
 * @brief Print addresses of nodes contained in the list.
 *
 * @param lst list of unvisited nodes.
 */
void print_slist(sys_slist_t * lst);

#endif
#endif 
