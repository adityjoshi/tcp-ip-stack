#include <assert.h>
#include "graph.h"
#include "net.h"

bool_t node_set_loopback_address(node_t *node, char*ip_addr ) {
assert(ip_addr);
node->node_network_prop->is_loopback_address_config = TRUE;
strncpy(NODE_LOOPBACKADDRESS(node),ip_addr,16);
NODE_LOOPBACKADDRESS(node)[15] = '\0';
}
bool_t node_set_interface_ip_address(node_t *node,char*local_if, char*ip_addr, char mask );
bool_t node_unset_interface_ip_address(node_t *node, char*local_if );

 
