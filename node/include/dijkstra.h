#ifndef DIJKSTRA_H
#define DIJKSTRA_H

#include "../include/graph.h"

#define INF 0xFFFF

// Error code returned by dijkstra_shortest_path that is not critical and just
// means that there is no connection found between nodes.
#define ENOPATH 150 

struct node_container {
	sys_snode_t next_container_node_ptr;
	struct node_t *node;
};

// PZIE: These could, actually, use a language check. But it's nice we have comments!
/**
 * @brief Use Dijkstra's algorithm to find the node that will be next hop for a packet. 
 * Both start and destination nodes should be in the same graph data structure.
 *
 * @param graph][] - array containing all nodes in the network.
 * @param graph_size - length of the graph array.
 * @param start_node - pointer to the start node.
 * @param dst_node - pointer to destination node.
 *
 * @return - Mesh id of the next node or < 0 on failure.
 */
int dijkstra_shortest_path(struct node_t graph[],
			   uint8_t graph_size,
			   struct node_t *start_node, struct node_t *dst_node);

#endif
