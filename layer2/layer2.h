#ifndef __LAYER2__
#define __LAYER2__


#include "net.h"
#include <stdlib.h>
#include "graph.h"
#include "utils.h"
#include "tcpconst.h"

#pragma pack (push,1) // to avoid padding done by the compiler 


typedef struct arpheader_ {
    short hardware_type; /*1 for ethernet cable*/
    short protocol_type; /*0x0800 for ipv4*/
    char hardwareaddr_len; /*6 for mac */
    char protocoladdr_len; /*4 for ipv4 */
    short op_code; /* req or reply */
    mac_address_t sender_mac;
    unsigned int src_ip;
    mac_address_t destination_mac;
    unsigned int dest_ip; /*ip for which arp is resolved */
    

} arpheader_t;


typedef struct ethernetHeader_ {
    mac_address_t dest;
    mac_address_t src;
    unsigned short type;
    char payload[248];
    unsigned int FCS; // crc
} ethernetHeader_t;

#pragma pack(pop) 


// ARP table 

typedef struct arp_table_{
    glthread_t arp_entries; 
} arp_table_t ; 


typedef struct arp_entries_ arp_entries_t;
typedef struct arp_entries_ {
ip_address_t ip_address;
mac_address_t mac_address;
char oif_name[IF_NAME_SIZE];
glthread_t arp_glue;
}  ;

GLTHREAD_TO_STRUCT(arp_glue_to_arp_entry, arp_entries_t, arp_glue);


/*
                API's for the ARP table

*/
void init_arp_table(arp_table_t **arp_table);

bool_t arp_table_entry_addition(arp_table_t *arp_table, arp_entries_t *arp_entry);

arp_entries_t * arp_table_entry_lookup(arp_table_t *arp_table, char *ip_addr);

void delete_arp_entry(arp_table_t *arp_table, char *ip_addr);

void
arp_table_update_from_arp_reply(arp_table_t *arp_table, 
    arpheader_t *arp_hdr, interface_t *iif);

void dump_arp_table(arp_table_t *arp_table);






/*
To check if the we can accept the packet or not arrived on the interface working in the layer 3 mode
*/
static inline bool_t l2_frame_recv_qualify_on_interface(interface_t *interface, ethernetHeader_t *ethernetHeader) {
    if(!IS_INTF_L3_MODE(interface)) {
        return FALSE;
    }
    if (memcpy(INTERFACE_MAC(interface),ethernetHeader->dest.mac_address,sizeof(mac_address_t)) == 0) {
        return TRUE;
    }
    if (IS_MAC_BROADCAST_ADDR(ethernetHeader->dest.mac_address)) {
        return TRUE ;
    }

    return FALSE ;

}


/*
*
*           VLAN SUPPORT 
*
*/

#pragma pack (push,1) // to avoid padding done by the compiler

typedef struct vlan_8021q_hdr_ {
    unsigned short tpid; /* = 0x8100*/
    short tci_pcp : 3; /* inital 4 bits not used*/
    short tci_dei : 1;  /*Not used*/
    short tci_vid : 12; /*tagged vlan id */
} vlan_8021q_hdr_t;

typedef struct vlan_ethernet_hdr_{

    mac_address_t dst_mac;
    mac_address_t src_mac;
    vlan_8021q_hdr_t vlan_8021q_hdr;
    unsigned short type;
    char payload[248];  /*Max allowed 1500*/
    unsigned int FCS;
} vlan_ethernet_hdr_t;




#pragma pack(pop)




static inline vlan_8021q_hdr_t * is_pkt_vlan_tagged(ethernetHeader_t *ethernet_hdr) {


vlan_8021q_hdr_t *vlan_8021q_hdr = (vlan_8021q_hdr_t *)((char *)ethernet_hdr + (sizeof(mac_address_t) * 2));

if(vlan_8021q_hdr->tpid == VLAN_8021Q_PROTO) {
    return vlan_8021q_hdr;
} else {
    return NULL;
}

}



static inline unsigned int GET_802_1Q_VLAN_ID(vlan_8021q_hdr_t *vlan_8021q_hdr) {
    return (unsigned int)vlan_8021q_hdr->tci_vid;
}



/*
*
*
*
MACROS 
*
*
*
*/



#define ETH_HDR_SIZE_EXCL_PAYLOAD   \
    (sizeof(ethernetHeader_t) - sizeof(((ethernetHeader_t *)0)->payload))

#define ETH_FCS(eth_hdr_ptr, payload_size)  \
    (*(unsigned int *)(((char *)(((ethernetHeader_t *)eth_hdr_ptr)->payload) + payload_size)))
#endif /* __LAYER2__ */


#define VLAN_ETH_FCS(vlan_eth_hdr_ptr, payload_size) \
    (*(unsigned int *)(((char *)(((vlan_ethernet_hdr_t *)vlan_eth_hdr_ptr)->payload)+payload_size)))


#define VLAN_ETH_HDR_SIZE_EXCL_PAYLOAD \
    (sizeof(vlan_ethernet_hdr_t) - sizeof(((vlan_ethernet_hdr_t *)0)->payload))



#define GET_COMMON_ETH_FCS(eth_hdr_ptr, payload_size) \
(is_pkt_vlan_tagged(eth_hdr_ptr) ? VLAN_ETH_FCS(eth_hdr_ptr, payload_size) : ETH_FCS(eth_hdr_ptr, payload_size))



static inline char *GET_ETHERNET_HEADER_PAYLOAD(ethernetHeader_t *ethernet_header) {
    return ethernet_header->payload;
}


static inline char *GET_ETHERNET_HDR_PAYLOAD(ethernetHeader_t *ethernet_hdr) {
    if (is_pkt_vlan_tagged(ethernet_hdr)) {
        return ((vlan_ethernet_hdr_t *)(ethernet_hdr))->payload;
    } else {
        return ethernet_hdr->payload;
    }
}



static inline void SET_COMMON_ETH_FCS(ethernetHeader_t *ethernet_hdr,  unsigned int payload_size,  unsigned int new_fcs) {
    if (is_pkt_vlan_tagged(ethernet_hdr)) {
        VLAN_ETH_FCS(ethernet_hdr,payload_size) = new_fcs ; 
    } else {
        ETH_FCS(ethernet_hdr,payload_size) = new_fcs ; 
    }
}


static inline unsigned int GET_ETH_HDR_SIZE_EXCL_PAYLOAD(ethernetHeader_t *ethernet_hdr) {
    if (is_pkt_vlan_tagged(ethernet_hdr)) {
        return VLAN_ETH_HDR_SIZE_EXCL_PAYLOAD;
    } else {
        return ETH_HDR_SIZE_EXCL_PAYLOAD;
    }
}


                 


static inline ethernetHeader_t * ALLOC_ETH_HDR_WITH_PAYLOAD(char *pkt, unsigned int pkt_size){ 
    char *temp = (char *)calloc(1, pkt_size);   
    memcpy(temp, pkt, pkt_size);
    ethernetHeader_t *eth_hdr = (ethernetHeader_t *)(pkt-ETH_HDR_SIZE_EXCL_PAYLOAD);
    memset((char *)eth_hdr, 0, ETH_HDR_SIZE_EXCL_PAYLOAD);
    memcpy(eth_hdr->payload, temp, pkt_size);
    SET_COMMON_ETH_FCS(eth_hdr, pkt_size, 0);
    free(temp);
    return eth_hdr;
}


static inline void
SET_COMMON_ETH_FCS(ethernetHeader_t *ethernet_hdr, 
                   unsigned int payload_size,
                   unsigned int new_fcs){
  
        ETH_FCS(ethernet_hdr, payload_size) = new_fcs;
    
}


void
node_set_intf_l2_mode(node_t *node, char *intf_name, intf_l2_mode_t intf_l2_mode);





ethernetHeader_t *tag_pkt_with_vlan_id(ethernetHeader_t *ethernet_hdr, unsigned int total_pkt_size, int vlan_id,  unsigned int *new_pkt_size);
                    


                     