#ifndef __NET__
#define __NET__


// add interface network properties and node network properties in graph.h 
#include <_string.h>
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





#endif
