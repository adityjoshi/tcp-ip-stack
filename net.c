#include <assert.h>
#include <string.h>
#include "gluethread/glthread.h"
#include "graph.h"
#include "net.h"
#include "utils.h"
#include <stdio.h>

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

}


bool_t node_set_loopback_address(node_t *node, char*ip_addr ) {
assert(ip_addr);
node->node_network_prop.is_loopback_address_config = TRUE;
strncpy(NODE_LOOPBACKADDRESS(node),ip_addr,16);
NODE_LOOPBACKADDRESS(node)[15] = '\0';
return TRUE ; 
}


bool_t node_set_interface_ip_address(node_t *node,char *local_if, char *ip_addr, char mask ) {
interface_t *interface = get_node_if_by_name(node, local_if);
 if(!interface) assert(0);
 strncpy(INTERFACE_IP(interface),ip_addr,16);
INTERFACE_IP(interface)[15] = '\0';
interface-> interface_nw_props.mask = mask; 
interface->interface_nw_props.is_ip_address_config = TRUE ;
return TRUE ; 
}
bool_t node_unset_intf_ip_address(node_t *node, char *local_if){

    return TRUE;
}



/*
 *
 * return the pointer to local interface 
 *
 *
 * */

interface_t *node_get_matching_subnet_interface(node_t *node, char *ip_addr)  {

unsigned int i = 0 ; 
interface_t *int_f;

char *intf_addr = NULL ; 
char mask ; 
char intf_subnet[16];
char subnet2[16];

for ( ; MAX_INTF_PER_NODE; i++) {
int_f = node->intf[i];

if (int_f->interface_nw_props.is_ip_address_config == FALSE) {
continue ; 
}
intf_addr = INTERFACE_IP(int_f);
}
}


void dump_nw_graph(graph_t *graph) {
node_t *node;
glthread_t *curr;
interface_t *interface;
unsigned int i ; 
printf("Topology Name = %s\n", graph->topology_name);

ITERATE_GLTHREAD_BEGIN(&graph->node_list, curr) {
node = graph_glue_to_node(curr);
dump_node_nw_props(node) ; 
for (i = 0 ; i<MAX_INTF_PER_NODE; i++) {
interface = node->intf[i];
if (!interface) break;
dump_intf_props(interface);
}
}ITERATE_GLTHREAD_END(&graph->node_list, curr);

} 



void dump_intf_props(interface_t *interface){

    dump_interface(interface);

    if(interface->interface_nw_props.is_ip_address_config){
        printf("\t IP Addr = %s/%u", INTERFACE_IP(interface), interface->interface_nw_props.mask);
        printf("\t MAC : %u:%u:%u:%u:%u:%u\n", 
                INTERFACE_MAC(interface)[0], INTERFACE_MAC(interface)[1],
                INTERFACE_MAC(interface)[2], INTERFACE_MAC(interface)[3],
                INTERFACE_MAC(interface)[4], INTERFACE_MAC(interface)[5]);
    }
}


void dump_node_nw_props(node_t * node) {
    printf("Node Name = %s\n", node->node_name);
    if(node->node_network_prop.is_loopback_address_config) {
        printf("\t Loopback IP : %s\n", NODE_LOOPBACKADDRESS(node));
    }
      printf("\n");
}
