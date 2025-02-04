#include "graph.h"
#include <malloc/_malloc.h>
#include <stdio.h>
#include <string.h>
#include "gluethread/glthread.h"


graph_t *create_new_graph(char *topology_name) {
graph_t *graph = calloc(1, sizeof(graph_t));
strncpy(graph->topology_name, topology_name , 32);
graph->topology_name[31]='\0';
init_glthread(&graph->node_list);
return graph ; 
}

void insert_link_between_node(node_t *node1, node_t *node2, char *from_interface_name, char *to_interface_name, unsigned int cost) {
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

/*
* Add connection for the node to which the link is connected i.e the attached node 
*/
link->intf1.att_node=node1;
link->intf2.att_node=node2;
link->cost= cost;

int empty_inf_slot ;

/*
* Store the plugin info for the node ; 
* */

empty_inf_slot = get_node_intf_available_slot(node1);
node1->intf[empty_inf_slot] = &link->intf1;
empty_inf_slot = get_node_intf_available_slot(node2);
node2->intf[empty_inf_slot] = &link->intf2;

}


 node_t *create_new_node(graph_t * graph, char *node_name) {
 node_t *node = calloc(1, sizeof(node_t));
strncpy(node->node_name,node_name,NODE_NAME_SIZE);
node->node_name[NODE_NAME_SIZE-1]='\0';
init_glthread(&node->graph_glue);
glthread_add_next(&graph->node_list,&node->graph_glue);
return node ; 

}


void dump_graph(graph_t *graph){

    node_t *node;
    glthread_t *curr;
    
    printf("Topology Name = %s\n", graph->topology_name);

    ITERATE_GLTHREAD_BEGIN(&graph->node_list, curr){

        node = graph_glue_to_node(curr);
        dump_node(node);    
    } ITERATE_GLTHREAD_END(&graph->node_list, curr);
}
void dump_node(node_t *node){

    unsigned int i = 0;
    interface_t *intf;

    printf("Node Name = %s : \n", node->node_name);
    for( ; i < MAX_INTF_PER_NODE; i++){
        
        intf = node->intf[i];
        if(!intf) break;
        dump_interface(intf);
    }
}

void dump_interface(interface_t *interface){

   link_t *link = interface->link;
   node_t *nbr_node = get_nbr_node(interface);

   printf("Interface Name = %s\n\tNbr Node %s, Local Node : %s, cost = %u\n", 
            interface->if_name,
            nbr_node->node_name, 
            interface->att_node->node_name, 
            link->cost);
           
}

