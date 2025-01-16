#include <assert.h>
#include <string.h>
#include "graph.h"
#include "net.h"

bool_t node_set_loopback_address(node_t *node, char*ip_addr ) {
assert(ip_addr);
node->node_network_prop->is_loopback_address_config = TRUE;
strncpy(NODE_LOOPBACKADDRESS(node),ip_addr,16);
NODE_LOOPBACKADDRESS(node)[15] = '\0';
return TRUE ; 
}
bool_t node_set_interface_ip_address(node_t *node,char*local_if, char*ip_addr, char mask ) {
interface_t *interface = get_node_if_by_name(node, local_if);
if (!interface) assert(0);
strncpy(INTERFACE_IP(interface),ip_addr,16);
INTERFACE_IP(interface)[15] = '\0';
interface-> interface_nw_props.mask = mask; 
interface->interface_nw_props.is_ip_address_config = TRUE ;
return TRUE ; 
}

 
