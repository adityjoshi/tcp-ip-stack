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

void *insert_link_between_node(node_t *node1, node_t *node2, char *from_interface_name, char *to_interface_name, unsigned int cost) {
link_t *link = calloc(1, sizeof(link_t));
strncpy(link->intf1.if_name,from_interface_name,IF_NAME_SIZE);
link->intf1.if_name[IF_NAME_SIZE-1] = '\0';
strncpy(link->intf2.if_name,to_interface_name,IF_NAME_SIZE);
link->intf2.if_name[IF_NAME_SIZE-1] = '\0';

/* set back pointer to the link
* this will help us to find the link between the child and parent 
* because of that we can find the neighbourhood node;
*
* */
link->intf1.link = link ;
link->intf2.link = link ;
};


 node_t *create_new_node(graph_t * graph, char *node_name) {
 node_t *node = calloc(1, sizeof(node_t));
strncpy(node->node_name,node_name,NODE_NAME_SIZE);
node->node_name[NODE_NAME_SIZE-1]='\0';
init_glthread(&node->graph_glue);
glthread_add_next(&graph->node_list,&node->graph_glue);
return node ; 
}
