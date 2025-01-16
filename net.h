#ifndef __NET__
#define __NET__
#include <memory.h>
#include <_string.h>

typedef struct node_ node_t ; 

/**
 *    TO DO 
 *     add interface network properties and node network properties in graph.h 
 */ 
typedef struct ip_address_ {
  char ip_address[16];
} ip_address_t ; 

typedef struct mac_address_ {
  char mac_address[8]; 
} mac_address_t; 


typedef struct node_nw_properties_ {
  // layer 3 config 
  bool_t is_loopback_address_config ; 
  ip_address_t loopback_addr;
} node_nw_properties_t;


static inline void 
init_node_nw_properties_(node_nw_properties_t *node_nw_prop) {
  node_nw_prop->is_loopback_address_config = FALSE; 
  memset(node_nw_prop->loopback_addr.ip_address,0,16);
}


typedef struct interface_nw_properties_ {
  /* L2 properties */
  mac_address_t mac_add;
  /* L3 properties */
  bool_t is_ip_address_config;
  ip_address_t ip_add;
  char mask ; 
} interface_nw_properties_t ; 


static inline void 
init_interface_nw_properties_(interface_nw_properties_ *interface_nw_prop) {
  memset(interface_nw_prop->mac_add, 0, 48);
  interface_nw_prop->is_ip_address_config = FALSE ; 
  memset(interface_nw_prop->ip_add, 0, 16);
  interface_nw_prop->mask = 0 ; 
};


/*
 * MACROS 
 */

#define INTERFACE_MAC(intf_ptr) ((intf_ptr)->interface_nw_props.mac_add.mac_address)
#define INTERFACE_IP(intf_ptr) ((intf_ptr)->interface_nw_props.ip_add.ip_address)
#define NODE_LOOPBACKADDRESS(node_ptr) ((node_ptr->ptr)->node_network_prop.loopback_addr.ip_address)


/*
 *
 * network apis 
 */

bool_t node_set_loopback_address(node_t *node, char*ip_addr );
bool_t node_set_interface_ip_address(node_t *node,char*local_if, char*ip_addr, char mask );
bool_t node_unset_interface_ip_address(node_t *node, char*local_if );

#endif /* __NET__ */
