#include "../include/dijkstra.h"
#include "../include/graph_api_generated.h"
#include <string.h>
#include <stdlib.h>


/**
 * @brief Gets a pointer to conatiner from unvisited list with the node that has
 * smallest tentative distance.
 *
 * @param lst - pointer to sys_slist_t data structure.
 * @param container_buffer - return buffer.
 *
 * @return - status code, -EINVAL on failure or 0 on success.
 */
static int get_smallest_td_node(sys_slist_t * lst,
			     struct node_container **container_buffer);

/**
 * @brief Visit neighboring nodes of the current node one by one and calculate the distance 
 * to them through the current node. If it is smaller than the current tentative_distance of 
 * the neighbor, update it.
 *
 * @param current_node - pointer to the node of which neighbors should be checked.
 *
 * @return - 0 on success and other on failure.
 */
static int recalculate_td_for_neighbours(struct node_t *current_node);

/**
 * @brief After calculating tentative_distances with Dijkstra algorithm, 
 * analyze graph and trace back path fomr the dst node to start node,
 * following the path of lowest tentative_distances.
 *
 * @param start_node - pointer to starting node.
 * @param dst_node - pointer to destination node.
 * @param paths_len - return buffer storing the length of the path. 
 *
 * @return - malloced array of consequent nodes mesh ids. It is user's 
 * responsibility to call free(). On failure NULL.
 */
static uint8_t *trace_back(struct node_t *start_node, struct node_t *dst_node, uint8_t * paths_len);


/**
 * @brief Create a singly linked list of nodes in the graph that were not 
 * visited yet during the Dijkstra algorithm.
 *
 * @param graph - array containing all nodes in the network.
 * @param graph_size - size of the graph.
 * @param lst - pointer to uninitialized list that will be initialized.
 *
 * @return - 0 on success != 0 on failure.
 */
static int create_unvisited_slist(struct node_t graph[],
			       uint8_t graph_size, sys_slist_t * lst);


/**
 * @brief At the end of the Dijkstra algorithm free malloced for slist memory.
 *
 * @param lst - list to be freed.
 */
static void free_slist(sys_slist_t * lst);


/**
 * @brief Remove a node that was visited from the unvisited list.
 *
 * @param lst - pointer to unvisited list.
 * @param node_to_remove - pointer to pointer of type node container to remove.
 */
static void remove_unvisited_slist_member(sys_slist_t * lst,
				   struct node_container **node_to_remove);


int dijkstra_shortest_path(struct node_t graph[],
			   uint8_t graph_size,
			   struct node_t *start_node, struct node_t *dst_node)
{
	int err;
    if(!graph || !start_node || !dst_node) return -EINVAL;

	// check if right node was picked
	if (!start_node->reserved || dst_node->reserved) {
		printk("ERROR: Node address not used!\n");
		printk("Provided addresses %d and %d\n", start_node->addr, dst_node->addr);
		return -EINVAL;
	}
	if (start_node->addr > MAX_MESH_SIZE || start_node->addr < 0
	    || dst_node->addr > MAX_MESH_SIZE || dst_node->addr < 0) {
		printk("Incorrect search address\n");
		return -EINVAL;
	}

	// set tentative_distance to 0 at start node
	err = node_t_tentative_distance_set(start_node, 0);
	if (err) {
		printk("ERROR: Could not set tentative distance\n");
		return err;
	}

	sys_slist_t lst;
	err = create_unvisited_slist(graph, graph_size, &lst);
    if(err) {
        printk("ERROR: Could not create list of unvisited nodes\n");
        return err;
    }

	// get smallest_td node and process it in a loop
	int smallest_td_node_found_code;
	struct node_container *smallest_td_node_container;
	while (!(smallest_td_node_found_code =
		 get_smallest_td_node(&lst, &smallest_td_node_container))) {
		if (!smallest_td_node_container) {
			printk("Smallest td node container is NULL!\n");
			return -EINVAL;
		}
		// visit a smallest_td node and update its neighbours td
        err = recalculate_td_for_neighbours(smallest_td_node_container->node);
        if (err) return err;

        // If found destination
		if (smallest_td_node_container->node->addr == dst_node->addr) {
			free_slist(&lst);
			break;
		}
		// remove processed node from a list
		remove_unvisited_slist_member(&lst,
					      &smallest_td_node_container);
	}

	// trace back and record a path
	uint8_t paths_size;
	uint8_t *path;
	path = trace_back(start_node, dst_node, &paths_size);

	if (!path) {
		printk("ERROR: Path was not found\n");
		return -EINVAL;
	}
    
    // Visualize path
	printk("Path: \n");
	for (int i = paths_size - 1; i >= 0; i--) {
		printk("%d", path[i]);
        if(i != 0) {
            printk("->");
        }
        else {
            printk("\n");
        }
	}

	// make all tentative_distances to INF again for future calculations
	err = reset_td_visited(graph, MAX_MESH_SIZE);
    __ASSERT(err == 0, "ERROR: Could not reset tentative_distances in the graph (err %d)\n", err);

	// path is in reverse order, so next node from current one is
	// one before last one (last is start node)
	int next_node_id = path[paths_size - 2];
	k_free(path);
	return next_node_id;
}

int get_smallest_td_node(sys_slist_t * lst,
			     struct node_container **container_buffer)
{
    int err;
	uint16_t smallest_td = INF;
	struct node_container *iterator;
    
    if(!lst || !container_buffer) return -EINVAL;

	*container_buffer = NULL;
	if (sys_slist_is_empty(lst)) {
		return -EINVAL;
	}

	// find lowest_td_node
	SYS_SLIST_FOR_EACH_CONTAINER(lst, iterator, next_container_node_ptr) {
		uint16_t td;
		err = node_t_tentative_distance_get(iterator->node, &td);
        if(err) return err;
		
        if (td <= smallest_td) {
			smallest_td = td;
			*container_buffer = iterator;
		}
	}

	if (!container_buffer) return -EINVAL;
    
	return 0;
}

static int recalculate_td_for_neighbours(struct node_t *current_node)
{
    int err;
    if(!current_node) return -EINVAL;
	for (uint8_t i = 0; i < current_node->paths_size; i++) {
		struct node_t *neighbour = (current_node->paths + i)->node_ptr;

		// consider only unvisited neighbours
		bool visited;
		err = node_t_visited_get(neighbour, &visited);
        if(err) return err;
		
        if (!visited) {
			struct path_t *path = current_node->paths + i;
			uint16_t cost_to_neighbour;
			err = path_t_cost_get(path, &cost_to_neighbour);
            if(err) return err;

			// check if distance through current node is smaller than neighbour's
			// current tentative distance
			uint16_t curr_node_td, neigh_td;
			err = node_t_tentative_distance_get(current_node,
						      &curr_node_td);
            if(err) return err;
			node_t_tentative_distance_get(neighbour, &neigh_td);
			if (curr_node_td + cost_to_neighbour < neigh_td) {
				err = node_t_tentative_distance_set(neighbour,
								    curr_node_td
								    +
								    cost_to_neighbour);
                if(err) return err;
			}
		}
	}
	err = node_t_visited_set(current_node, true);
    if(err) return err;
    return 0;
}

static uint8_t *trace_back(struct node_t *start_node, struct node_t *dst_node, uint8_t * paths_len)
{
    uint8_t index = 0;
    int err;
	struct node_t *current_node = dst_node;

    if(!start_node || !dst_node) return NULL;
	
    // array to store longest path possible, it will be realloc'ed in the end to correct size
	uint8_t *path = k_malloc(sizeof(uint8_t) * MAX_MESH_SIZE);
    __ASSERT(path != NULL, "ERROR: Could not allocate memory for path tracing\n");

	// get to node and check its neighbours for lowest td
	path[index] = current_node->addr;
	while (current_node != start_node) {
		// check which of neighbours has smallest td
		uint16_t smallest_td = INF;
		uint8_t number_of_paths = current_node->paths_size;
		struct node_t *temp_node = current_node;
		for (uint8_t i = 0; i < number_of_paths; i++) {
			struct node_t *checked_node =
			     ((current_node->paths) + i)->node_ptr;
			uint16_t checked_node_td;
			err = node_t_tentative_distance_get(checked_node,
						      &checked_node_td);
			if (checked_node_td <= smallest_td) {
				smallest_td = checked_node_td;
				temp_node = checked_node;	// here some temp node
			}
		}

        // Pick neighbor with smallest td and set it as current node
		// smallest td node address add to a path and set it as current node
		current_node = temp_node;
		index++;
		path[index] = current_node->addr;

		// If path was not found...
		if (index > MAX_MESH_SIZE) {
			k_free(path);

			*paths_len = 0;
			return NULL;
		}
	}

	*paths_len = index + 1;
    realloc(path, (*paths_len) * sizeof(uint8_t));
    printk("Memory %d\n", sizeof(uint8_t) * (*paths_len));
    __ASSERT(path != NULL, "ERROR: Could not reallocate memory for path tracing\n");
	return path;
}

static int create_unvisited_slist(struct node_t graph[],
			       uint8_t graph_size, sys_slist_t * lst)
{
    if(!graph || !lst) return -EINVAL;
	sys_slist_init(lst);
	for (uint8_t i = 0; i < graph_size; i++) {
		if (graph[i].reserved) {
			struct node_container *container =
			    k_malloc(sizeof(struct node_container));
            __ASSERT(container != NULL, "ERROR: Could not allocate memory for node container \n");
			container->node = graph + i;
			sys_slist_append(lst,
					 &container->next_container_node_ptr);
		}
	}
	return 0;
}

static void free_slist(sys_slist_t * lst)
{
	volatile sys_snode_t *to_remove;
	while ((to_remove = sys_slist_get(lst))) {
		struct node_container *container = CONTAINER_OF(to_remove,
								struct
								node_container,
								next_container_node_ptr);
		k_free(container);
	}
}

static void remove_unvisited_slist_member(sys_slist_t * lst, struct node_container
				   **node_container_to_remove)
{
	sys_slist_find_and_remove(lst,
				  &
				  (*node_container_to_remove)->next_container_node_ptr);
	k_free(*node_container_to_remove);
}
