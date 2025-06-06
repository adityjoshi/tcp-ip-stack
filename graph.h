#ifndef __GRAPH__
#define __GRAPH__

#include <assert.h>
#include "gluethread/glthread.h"
#include "net.h"
#include "string.h"


#define IF_NAME_SIZE 16
#define NODE_NAME_SIZE 16
#define MAX_INTF_PER_NODE 10 

// forwarad declaration 
typedef struct node_ node_t ;
typedef struct link_ link_t;
typedef struct interface_ interface_t ;

typedef struct interface_ {
  char if_name[IF_NAME_SIZE];
  struct node_ *att_node; // att_node -> attached node 
  struct link_ *link;
  interface_nw_properties_t interface_nw_props ; 

} interface_t;

struct link_ {
  interface_t intf1;
  interface_t intf2; 
  unsigned int cost; 
};
 
struct node_ {
  char node_name[NODE_NAME_SIZE];
  interface_t *intf[MAX_INTF_PER_NODE];
  glthread_t graph_glue;
  unsigned int udp_port_number;
  int udp_socket_fd;

  node_nw_properties_t node_network_prop ; 

};

GLTHREAD_TO_STRUCT(graph_glue_to_node, node_t, graph_glue);

typedef struct graph_{

    char topology_name[32];
    glthread_t node_list; 
} graph_t;


node_t *
create_new_node(graph_t *graph, char *node_name);

graph_t *
create_new_graph(char *topology_name);

void
insert_link_between_node(node_t *node1, 
                             node_t *node2,
                             char *from_if_name, 
                             char *to_if_name, 
                             unsigned int cost);


/*
 * Helper functions 
 */
static inline node_t *get_nbr_node(interface_t *interface) {

  assert(interface->att_node);
  assert(interface->link);
  /*
   * there will be two cases either it will point to the interface 1 or the interface 2 
   */
  link_t *link = interface->link;
  if(&link->intf1 == interface) {
    return link->intf2.att_node;
  } else {
    return link->intf1.att_node;
  }
}


static inline int get_node_intf_available_slot(node_t *node) {
  for (int i = 0 ; i < MAX_INTF_PER_NODE; i++) {
    if(node->intf[i])
            continue;
        return i;
  }
  return -1;
}


/*
Fucntion which returns pointer to the local interface of a node, searched searched by if_name.

*/
static inline interface_t * get_node_if_by_name(node_t *node, char *if_name) {
  interface_t *interface;
  for (int i = 0; i<MAX_INTF_PER_NODE; i++) {
    interface = node->intf[i];
    if(!interface) continue ; 
    if(strncmp(interface->if_name,if_name,IF_NAME_SIZE)==0) {
      return interface; 
    } 
   
  }
   return NULL; 
}

/*
This function returns pointer to the node present in a graph list, searched by node name.
*/
static inline node_t *
get_node_by_node_name(graph_t *topo, char *node_name) {
  node_t *node;
  glthread_t *glthread;
  ITERATE_GLTHREAD_BEGIN(&topo->node_list,glthread) {
    node = graph_glue_to_node(glthread);
    if(strncmp(node->node_name, node_name, strlen(node_name)) == 0)
      return node ; 
     ITERATE_GLTHREAD_END(&topo->node_list, curr);
    return NULL ; 
    
  }
}

void dump_graph(graph_t *graph);
void dump_node(node_t *node);
void dump_interface(interface_t *interface);

#endif // /* __NW_GRAPH_ */

