#ifndef __GRAPH__
#define __GRAPH__

#include <assert.h>
#include "gluethread/gltthread.h"



typedef struct interface_ {
  char if_name[IF_NAME_SIZE];
  struct node_ *att_node;
  struct link_ *link;

} interface_t;

struct link_ {
  interface_t intf1;
  interface_t intf2; 
  unsigned int cost; 
};

struct node_ {
  char node_name[NODE_NAME_SIZE];
  interface_t *int_f[MAX_INTF_PER_NODE];
  glthread_t graph_glue;

}
typedef struct graph_{
  char topology_name[32];
  glthread_t node_list;
} graph_t;



#endif // /* __NW_GRAPH_ */

