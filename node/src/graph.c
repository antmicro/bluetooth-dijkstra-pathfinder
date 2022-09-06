#include "../include/graph.h"
#include "../include/dijkstra.h"
#include "kernel.h"
#include <bluetooth/bluetooth.h>
#include <math.h>
#include "../include/graph_api_generated.h"


/**
 * @brief Global variable that is set to point to self after intialization phase.
 */
struct node_t *common_self_ptr = NULL;

int reset_td_visited(struct node_t graph[], uint8_t size)
{
    if(!graph) return -EINVAL;
	for (uint8_t i = 0; i < size; i++) {
		if ((graph + i)->reserved) {
			node_t_tentative_distance_set(graph + i, INF);
			node_t_visited_set(graph + i, false);
		}
	}
    return 0;
}

// Setters and getters respecting the mutex access
int node_t_visited_set(struct node_t *node, bool new_val)
{
	int err;
    if(!node) return -EINVAL;
	err = k_mutex_lock(&node->node_mutex, K_FOREVER);
	if (err)
		return err;
	node->visited = new_val;
	k_mutex_unlock(&node->node_mutex);
	if (err)
		return err;
	return 0;
}

int node_t_visited_get(struct node_t *node, bool *ret_val)
{
	int err;
    if(!node || !ret_val) return -EINVAL;
	err = k_mutex_lock(&node->node_mutex, K_FOREVER);
	if (err)
		return err;
	*ret_val = node->visited;
	k_mutex_unlock(&node->node_mutex);
	if (err)
		return err;
	return 0;
}

int node_t_tentative_distance_set(struct node_t *node, uint16_t new_val)
{
	int err;
    if(!node) return -EINVAL;
	err = k_mutex_lock(&node->node_mutex, K_FOREVER);
	if (err)
		return err;
	node->tentative_distance = new_val;
	k_mutex_unlock(&node->node_mutex);
	if (err)
		return err;
	return 0;
}

int node_t_tentative_distance_get(struct node_t *node, uint16_t * ret_val)
{
	int err;
    if(!node || !ret_val) return -EINVAL;
	err = k_mutex_lock(&node->node_mutex, K_FOREVER);
	if (err)
		return err;
	*ret_val = node->tentative_distance;
	k_mutex_unlock(&node->node_mutex);
	if (err)
		return err;
	return 0;
}

int path_t_cost_set(struct path_t *path, uint16_t new_val)
{
	int err;
    if(!path) return -EINVAL;
	err = k_mutex_lock(&path->path_mutex, K_FOREVER);
	if (err)
		return err;
	path->cost = new_val;
	err = k_mutex_unlock(&path->path_mutex);
	if (err)
		return err;
	return 0;
}

int path_t_cost_get(struct path_t *path, uint16_t * ret_val)
{
	int err;
    if(!path || !ret_val) return -EINVAL;
	err = k_mutex_lock(&path->path_mutex, K_FOREVER);
	if (err)
		return err;
	*ret_val = path->cost;
	err = k_mutex_unlock(&path->path_mutex);
	if (err)
		return err;
	return 0;
}

int graph_set_cost_uni_direction(struct node_t *node1, struct node_t *node2, uint8_t new_cost)
{
    int err;
	for (uint8_t i = 0; i < node1->paths_size; i++) {
		if ((node1->paths + i)->node_ptr == node2) {
			err = path_t_cost_set(node1->paths + i, new_cost);
            if(err) return err;
            return 0;
		}
	}
    return EINVAL;
}


int node_to_byte_array(struct node_t *node, uint8_t buffer[],
			uint8_t buffer_size)
{
	// First two fields in buffer are always the same and identify the node 
	// with relation to which the rest of connections is made
    if(!node || !buffer) return -EINVAL;

	buffer[0] = node->addr;
	buffer[1] = node->paths_size;

	// Rest of fields represent the connections of that node
	for (uint8_t i = 0; i < node->paths_size; i++) {
		uint8_t neighbor_addr_idx = 2 * i + 2 + 0;
		uint8_t neighbor_dist_idx = 2 * i + 2 + 1;
        
        if(neighbor_dist_idx > node->paths_size - 1 || neighbor_dist_idx > node->paths_size - 1) {
            return -EINVAL;
        }

		uint16_t cost;
		path_t_cost_get(node->paths + i, &cost);
		buffer[neighbor_addr_idx] = (node->paths + i)->node_ptr->addr;
		buffer[neighbor_dist_idx] = cost;
	}
    return 0;
}


int load_rtr(struct node_t graph[], uint8_t buff[], uint8_t size)
{
    if(!graph || !buff) return -EINVAL;
	// First two bytes of each rtr are self addr and n neighs
	uint8_t node_addr = buff[0];
	uint8_t neighs_n = buff[1];
    
	// Then iterate over node neighs 
	for (uint8_t j = 0; j < neighs_n; j++) {
		uint8_t idx = 1 + 1 + (2 * j);
		if (idx > size) {
			return EINVAL;
		}
		uint8_t neigh_addr = buff[idx];
		uint8_t cost_to_neigh = buff[idx + 1];
        int err = graph_set_cost_uni_direction(&graph[node_addr], &graph[neigh_addr], cost_to_neigh);
        if(err) return err;
	}
    return 0;
}

void print_graph(struct node_t graph[])
{
    __ASSERT(graph != NULL, "ERROR: Graph is NULL\n");
	printk("Mesh topology:\n");
	uint16_t cost;
	for (uint8_t i = 0; i < MAX_MESH_SIZE; i++) {
		printk("Node %d\n", graph[i].addr);
		printk("Connected to: \n");
		for (uint8_t j = 0; j < graph[i].paths_size; j++) {
			path_t_cost_get(graph[i].paths + j, &cost);
			printk("----Node %d with distance: %d \n",
			       graph[i].paths[j].node_ptr->addr, cost);
		}
	}
}

int identify_self_in_graph(struct node_t *graph, char identity_str[], uint8_t len)
{
    if(!graph || !identity_str) return -EINVAL;
	// get all configured identities 
	bt_addr_le_t identities[CONFIG_BT_ID_MAX];
	size_t count = 0;
    
	bt_id_get(NULL, &count);	
    printk("count %d\n", count);
	bt_id_get(identities, &count);
    if(count == 0) return -EINVAL;

	bt_addr_le_to_str(&identities[0], identity_str, len);

	int err = get_ptr_to_node_by_ble_addr(graph, identity_str, &common_self_ptr);
    if(err) return err;
    return 0;
}


int get_ptr_to_node_by_ble_addr(struct node_t *graph,
				char *ble_addr, struct node_t **ptr)
{
    if(!graph || !ble_addr || !ptr) return -EINVAL;

	for (uint8_t i = 0; i < MAX_MESH_SIZE; i++) {
        printk("graph addr %s compared %s\n", graph[i].addr_bt_le, ble_addr);
		if (!memcmp(graph[i].addr_bt_le, ble_addr, 17)) {
			*ptr = (graph + i);
			return 0;
		}
	}
	return -EINVAL;
}
