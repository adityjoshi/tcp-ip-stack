#ifndef __NET__
#define __NET__


// add interface network properties and node network properties in graph.h 
typedef struct ip_address_ {
  char ip_address[16];
} ip_address_t ; 

typedef struct mac_address_ {
  char mac_address[8]; 
} mac_address_t; 


typedef struct node_nw_properties_ {
  // layer 3 config 
  bool_t is_loopback_address_config ; 
  ip_address_t lb_addr;
} node_nw_properties_t;






#endif
