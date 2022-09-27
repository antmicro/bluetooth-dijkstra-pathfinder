#ifndef GRAPH_H
#define GRAPH_H

#include <stdint.h>
#include <stdbool.h>
#include <kernel.h>
#include <bluetooth/addr.h>
#include "graph_api_generated.h"

struct node_t {
	struct k_mutex node_mutex;
	uint8_t addr;
	char addr_bt_le[18];
	bool reserved;

	bool visited;

	uint16_t tentative_distance;

	uint8_t paths_size;
	struct path_t *paths;
};

extern struct node_t *common_self_ptr;
extern struct node_t graph[MAX_MESH_SIZE];

uint8_t graph_init(struct node_t *graph);

// Dijkstra's utility
int reset_td_visited(struct node_t *graph, uint8_t size);

// Setters and getters respecting the mutex access
int node_t_visited_set(struct node_t *node, bool new_val);
int node_t_visited_get(struct node_t *node, bool *ret_val);
int node_t_tentative_distance_set(struct node_t *node, uint16_t new_val);
int node_t_tentative_distance_get(struct node_t *node, uint16_t * ret_val);
int path_t_cost_set(struct path_t *path, uint16_t new_val);
int path_t_cost_get(struct path_t *path, uint16_t * ret_val);

/**
 * @brief Set cost from one node to another. Works in one way i.e. modifies
 * only path from node1 to node2 but not the other way around. Nodes should not
 * too eagerly change the costs for their peers on the basis of their own
 * calculation, but rather wait for routing table record and read transition
 * costs of peers from it.
 *
 * @param node1 - pointer to node which will have it's paths modified with new cost.
 * @param node2 - pointer that will be searched in paths of node1 to which cost will be altered.
 * @param new_cost - new cost value.
 *
 * @return - error code, 0 on success and > on mutex failure or when no such
 * path bewteen specified nodes was found then EINVAL.
 */
int graph_set_cost_uni_direction(struct node_t *node1, struct node_t *node2,
				 uint16_t new_cost);

/**
 * @brief Convert node's address, paths and paths size into the byte array
 * suitable for sending with BLE.
 * @param node - pointer to node which will be sent as byte array.
 * @param buffer][] - buffer to store the byte array.
 * @param buffer_size - size of a buffer.
 *
 * @return - 0 on success and -EINVAL on failure.
 */
int node_to_byte_array(struct node_t *node, uint8_t buffer[],
		       uint8_t buffer_size);

/**
 * @brief Load information received from peer about it's connections, received as a byte array, to
 * current node own graph data structure.
 * Each buffer is encoded as:
 *   B1    |        B2       |      B3    |      B4    |      B5    |     B6
 *  addr   | number of peers | peer1 addr | peer1 cost | peer2 addr | peer2 cost
 *
 * @param graph[] - data structure to which information will be loaded.
 * @param buff[] - buffer with received data.
 * @param size - size of that buffer.
 *
 * @return - 0 on success and -EIN
 */
int load_rtr(struct node_t graph[], uint8_t buff[], uint8_t size);

/**
 * @brief Print the graph structure with connections in between nodes and
 * corresponding costs.
 *
 * @param graph[] - array of struct node_t.
 */
void print_graph(struct node_t graph[]);

/**
 * @brief Finds default identity of the device (BLE address), then checks in
 * the graph data structure for mesh id corresponding to that identity. It will
 * set the value of global pointer - common_self_ptr that points to the self in
 * the graph data structure.
 *
 * @param graph - Array with nodes.
 * @param identity_str[] - buffer that will be filled with identity string.
 * @param len - length of the buffer to fill. Should be at least 17 to handle
 * entire BLE address.
 *
 * @return - 0 on success and -EINVAL on failure.
 */
int identify_self_in_graph(struct node_t *graph, char identity_str[],
			   uint8_t len);

/**
 * @brief Iterate over graph data structure and set ptr to point to the node in
 * graph with matching ble_addr.
 *
 * @param graph - array of nodes.
 * @param ble_addr - valid BLE address matching one of the addresses in the graph
 * data structure.
 * @param ptr - pointer to pointer that will point to the resultant node.
 *
 * @return - 0 on success and -EINVAL on failure.
 */
int get_ptr_to_node_by_ble_addr(struct node_t *graph, char *ble_addr,
				struct node_t **ptr);

#endif
