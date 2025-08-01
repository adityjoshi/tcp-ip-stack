#include <assert.h>
#include <stdint.h>
#include <string.h>
#include "gluethread/glthread.h"

#include "net.h"
#include "graph.h"
#include "utils.h"
#include <stdio.h>
#include <arpa/inet.h> 
#include <sys/_endian.h>
#include <sys/socket.h>
#include <stdlib.h>

static unsigned int
hash_code(void *ptr, unsigned int size){
    unsigned long long value=0, i =0;
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
unsigned long long hashVal = 0 ; 
    hashVal = hash_code(node->node_name, NODE_NAME_SIZE);
    hashVal *= hash_code(interface->if_name, IF_NAME_SIZE);
    memset(INTERFACE_MAC(interface), 0, sizeof(INTERFACE_MAC(interface)));
    memcpy(INTERFACE_MAC(interface), (char *)&hashVal, sizeof(unsigned long long));

}


extern void rt_table_add_direct_route(rt_table_t *rt_table, char *ip_addr, char mask);


bool_t node_set_loopback_address(node_t *node, char*ip_addr ) {
assert(ip_addr);
node->node_network_prop.is_loopback_address_config = TRUE;
strncpy(NODE_LOOPBACKADDRESS(node),ip_addr,16);
NODE_LOOPBACKADDRESS(node)[15] = '\0';

rt_table_add_direct_route(Node_RT_TABLE(node), ip_addr, 32);
return TRUE ; 
}


bool_t node_set_interface_ip_address(node_t *node,char *local_if, char *ip_addr, char mask ) {

    interface_t *interface = get_node_if_by_name(node, local_if);
    if(!interface) assert(0);

    strncpy(INTERFACE_IP(interface), ip_addr, 16);
    INTERFACE_IP(interface)[15] = '\0';
    interface->interface_nw_props.mask = mask; 
    interface->interface_nw_props.is_ip_address_config = TRUE;
    rt_table_add_direct_route(Node_RT_TABLE(node), ip_addr, mask);
    return TRUE;
}


bool_t node_unset_intf_ip_address(node_t *node, char *local_if){
    return TRUE;
}



/*
 *
 * return the pointer to local interface 
 * For node R0 containing two local interfaces with ip/mask configured as : 40.1.1.1/24 on eth0/4 and 20.1.1.1/24 on eth0/0,  this API must return :
 * pointer to eth0/4 if ip_addr passed is 40.1.1.x, for all x [0-255]
 * pointer to eth0/0 if ip_addr passed is 20.1.1.x, for all x [0-255]
 *
 * */

interface_t *node_get_matching_subnet_interface(node_t *node, char *ip_addr)  {

unsigned int i = 0 ; 
interface_t *int_f;

char *intf_addr = NULL ; 
char mask ; 
char intf_subnet[16];
char subnet2[16];

for ( ;i< MAX_INTF_PER_NODE; i++) {
int_f = node->intf[i];
if(!int_f) {
continue ; 
}
if (int_f->interface_nw_props.is_ip_address_config == FALSE) {
continue ; 
}
intf_addr = INTERFACE_IP(int_f);
mask = int_f->interface_nw_props.mask ; 

memset(intf_subnet, 0, 16);
memset(subnet2, 0, 16);
apply_mask(intf_addr, mask, intf_subnet);
apply_mask(ip_addr,mask,subnet2);

if (strncmp(intf_subnet,subnet2,16) == 0) {
return int_f ; 
}
}
}

char *convert_ip_from_int_to_str(unsigned int ip_addr, char *output_buffer) {
char *out = NULL ; 
static char str_ip[16];
out = !output_buffer ? str_ip:output_buffer;
memset(out , 0, 16);
ip_addr = htonl(ip_addr);
inet_ntop(AF_INET,&ip_addr,out,16);
out[15] = '\0';
return out ; 
}

unsigned int convert_ip_from_str_to_int(char *ip_addr) {
uint32_t binary_prefix = 0 ; 
inet_pton(AF_INET,ip_addr, &binary_prefix) ;
binary_prefix = htonl(binary_prefix);
return binary_prefix ; 

}

/*

pkt = [A B C D E 0 0 0 0 0]  

Copy pkt to right:
pkt = [A B C D E A B C D E]

Clear first pkt_size bytes:
pkt = [0 0 0 0 0 A B C D E]

return pointer -> &pkt[5]

*/

char *
pkt_buffer_shift_right(char *pkt, unsigned int pkt_size,unsigned int total_buffer_size) {
    char *temp = NULL ;
    bool_t tempMemory = FALSE; 
    if (pkt_size * 2 > total_buffer_size) {
        tempMemory = TRUE;
    }
    if (tempMemory) {
        temp = calloc(1, pkt_size);
        memcpy(temp, pkt, pkt_size);
        memset(pkt, 0, total_buffer_size);
        memcpy(pkt + (total_buffer_size-pkt_size), temp, pkt_size);
        free(temp); 
        return pkt + (total_buffer_size-pkt_size);
    }
    memcpy(pkt + (total_buffer_size - pkt_size), pkt, pkt_size);
    memset(pkt, 0, pkt_size);
    return pkt + (total_buffer_size - pkt_size);

}





unsigned int get_access_intf_operating_vlan_id(interface_t *interface) {
    if (IF_L2_Mode(interface) != ACCESS) {
         assert(0) ; 
    }
    return interface->interface_nw_props.vlans[0];
}


bool_t is_trunk_interface_vlan_enabled(interface_t *interface, unsigned int vlan_id) {
    if (IF_L2_Mode(interface) != TRUNK) {
        assert(0);
    }
    for (unsigned int i = 0 ; i<MAX_VLAN_MEMBERSHIP; i++) {
        if (interface->interface_nw_props.vlans[i] == vlan_id) {
            return TRUE ; 
        }
    }
    return FALSE ; 
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
    } else {
        printf("\t l2 mode = %s", intf_l2_mode_str(IF_L2_Mode(interface)));
         printf("\t vlan membership : ");
         int i = 0;
         for(; i < MAX_VLAN_MEMBERSHIP; i++){
            if(interface->interface_nw_props.vlans[i]){
                printf("%u  ", interface->interface_nw_props.vlans[i]);
            }
         }
         printf("\n");
    }
}


void dump_node_nw_props(node_t * node) {
    printf("\nNode Name = %s, UDP Port No = %u\n", node->node_name, node->udp_port_number);
   
    if(node->node_network_prop.is_loopback_address_config) {
        printf("\t Loopback IP : %s\n", NODE_LOOPBACKADDRESS(node));
    }
      printf("\n");
}
