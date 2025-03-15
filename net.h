#ifndef __NET__
#define __NET__
#include <memory.h>
#include <_string.h>
#include "utils.h"



typedef struct graph_ graph_t;
typedef struct interface_ interface_t;
typedef struct node_ node_t ; 

/**
 *    TO DO : done 
 *     add interface network properties and node network properties in graph.h 
 */ 
typedef struct ip_address_ {
  unsigned char ip_address[16];
} ip_address_t ; 

typedef struct mac_address_ {
 unsigned char mac_address[6]; 
} mac_address_t; 

//forward declaration
typedef struct arp_table_ arp_table_t;
extern void init_arp_table(arp_table_t **arp_table);

typedef struct node_nw_properties_ {

  // layer 2 config 
  arp_table_t *arp_table; 
  // layer 3 config 
  bool_t is_loopback_address_config ; 
  ip_address_t loopback_addr;
} node_nw_properties_t;


static inline void 
init_node_nw_properties_(node_nw_properties_t *node_nw_prop) {
  node_nw_prop->is_loopback_address_config = FALSE; 
  memset(node_nw_prop->loopback_addr.ip_address,0,16);
  init_arp_table(&(node_nw_prop->arp_table));

}


typedef enum {
  ACCESS,
  TRUNK,
  L2_MODE_UNKNOWN
} intf_l2_mode_t;

static inline char *intf_l2_mode_str(intf_l2_mode_t intf_l2_mode) {
  switch(intf_l2_mode) {
    case ACCESS:
      return "access";
    case TRUNK:
      return "trunk";
    default:
      return "L2_mode_unknown";

  }
}


typedef struct interface_nw_properties_ {
  /* L2 properties */
  mac_address_t mac_add;
  intf_l2_mode_t intf_l2_mode; // if ip address is configured to this interface then it should be set to unknown state 
  
  /* L3 properties */
  bool_t is_ip_address_config;
  ip_address_t ip_add;
  char mask ; 
} interface_nw_properties_t ; 

void interface_assign_mac_address(interface_t *interface);

static inline void 
init_interface_nw_properties_(interface_nw_properties_t *interface_nw_prop) {
  memset(&interface_nw_prop->mac_add.mac_address, 0, sizeof(interface_nw_prop->mac_add.mac_address));
  interface_nw_prop->is_ip_address_config = FALSE ; 
  memset(&interface_nw_prop->ip_add, 0, 16);
  interface_nw_prop->mask = 0 ; 
};


/*
 * MACROS 
 */

#define INTERFACE_MAC(intf_ptr) ((intf_ptr)->interface_nw_props.mac_add.mac_address) // returns the pointer to the mac address of the interface 
#define INTERFACE_IP(intf_ptr) ((intf_ptr)->interface_nw_props.ip_add.ip_address) // returns the pointer to the ip address of the interface 
#define NODE_LOOPBACKADDRESS(node_ptr) ((node_ptr)->node_network_prop.loopback_addr.ip_address) // return the pointer to the IP address configured as loopback on a device
#define IS_INTF_L3_MODE(intf_ptr)     (intf_ptr->interface_nw_props.is_ip_address_config == TRUE) // returns true if the interface is in L3 mode
#define NODE_ARP_TABLE(node_ptr)    (node_ptr->node_network_prop.arp_table) // returns the pointer to the arp table of the node
#define IF_L2_Mode(intf_ptr) (intf_ptr->interface_nw_props.intf_l2_mode) // returns the L2 mode of the interface
/*
 *
 *
 *
 *
 */







/*
 *
 * network apis 
 */

bool_t node_set_loopback_address(node_t *node, char*ip_addr );
bool_t node_set_interface_ip_address(node_t *node,char*local_if, char*ip_addr, char mask );
bool_t node_unset_interface_ip_address(node_t *node, char*local_if );



/*
 *
 * Display apis
 *
 *
 * */

void dump_nw_graph(graph_t *graph);
void dump_node_nw_props(node_t *node);
void dump_intf_props(interface_t *interface);


interface_t *node_get_matching_subnet_interface(node_t *node, char *ip_addr) ;
unsigned int convert_ip_from_str_to_int(char *ip_addr);
char *convert_ip_from_int_to_str(unsigned int ip_addr, char *output_buffer);

char *
pkt_buffer_shift_right(char *pkt, unsigned int pkt_size,unsigned int total_buffer_size);


interface_t * node_get_matching_subnet_interface(node_t *node, char *ip_addr);
                               

#endif /* __NET__ */
