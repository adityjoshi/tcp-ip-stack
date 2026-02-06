#ifndef __PATHFINDER__
#define __PATHFINDER__

#include "graph.h"

typedef struct path_node_ path_node_t;

path_node_t *find_path(node_t *src, node_t *dst);
void print_path(path_node_t *path_end);
void free_path(path_node_t *path_end);
void find_and_configure_paths_to_node(node_t *dst_node);

#endif /* __PATHFINDER__ */
