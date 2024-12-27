#include "graph.h"
#include <malloc/_malloc.h>
#include <string.h>
#include "gluethread/gltthread.h"


graph_t *create_new_graph(char *topology_name) {
graph_t *graph = calloc(1, sizeof(graph_t));
strncpy(graph->topology_name, topology_name , 32);
graph->topology_name[31]='\0';
init_glthread(&graph->node_list);
return graph ; 
}


 node_t *create_new_node(graph_t * graph, char *node_name) {
 node_t *node = calloc(1, sizeof(node_t));
strncpy(node->node_name,node_name,NODE_NAME_SIZE);
node->node_name[NODE_NAME_SIZE]='\0';
init_glthread(&node->graph_glue);
glthread_add_next(&graph->node_list,&node->graph_glue);
return node ; 
}
