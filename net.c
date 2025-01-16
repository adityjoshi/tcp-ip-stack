#include <assert.h>
#include <string.h>
#include "graph.h"
#include "net.h"


static unsigned int
hash_code(void *ptr, unsigned int size){
    unsigned int value=0, i =0;
    char *str = (char*)ptr;
    while(i < size)
    {
        value += *str;
        value*=97;
        str++;
        i++;
    }
    return value;
}

void interface_assign_mac_address(interface_t *interface) {
node_t *node = interface->att_node;
if(!node) {
return ; 
}
unsigned int hashVal = 0 ; 
    hashVal = hash_code(node->node_name, NODE_NAME_SIZE);
    hashVal *= hash_code(interface->if_name, IF_NAME_SIZE);
    memset(INTERFACE_MAC(interface), 0, sizeof(INTERFACE_MAC(interface)));
    memcpy(INTERFACE_MAC(interface), (char *)&hashVal, sizeof(unsigned int));




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

 
