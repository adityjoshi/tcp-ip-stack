#ifndef __LAYER2__
#define __LAYER2__
#include "net.h"
#include <stdlib.h>
#include "graph.h"

#pragma pack (push,1) // to avoid padding done by the compiler 
typedef struct ethernetHeader_ {
    mac_address_t dest;
    mac_address_t src;
    unsigned short type;
    char payload[248];
    unsigned int FCS; // crc
} ethernetHeader_t;
#pragma pack(pop) 


/*
To check if the we can accept the packet or not arrived on the interface working in the layer 3 mode
*/
static inline bool_t l2_frame_recv_qualify_on_interface(interface_t *interface, ethernetHeader_t *ethernetHeader) {
    if(!IS_INTF_L3_MODE(interface)) {
        return FALSE;
    }

}










/*

MACROS 

*/
#define ETH_HDR_SIZE_EXCL_PAYLOAD   \
    (sizeof(ethernetHeader_t) - sizeof(((ethernetHeader_t *)0)->payload))

    #define ETH_FCS(eth_hdr_ptr, payload_size)  \
    (*(unsigned int *)(((char *)(((ethernetHeader_t *)eth_hdr_ptr)->payload) + payload_size)))
#endif /* __LAYER2__ */



static inline ethernetHeader_t * ALLOC_ETH_HDR_WITH_PAYLOAD(char *pkt, unsigned int pkt_size){ 
    char *temp = (char *)calloc(1, pkt_size);   
    memcpy(temp, pkt, pkt_size);
    ethernetHeader_t *eth_hdr = (ethernetHeader_t *)(pkt-ETH_HDR_SIZE_EXCL_PAYLOAD);
    memset((char *)eth_hdr, 0, ETH_HDR_SIZE_EXCL_PAYLOAD);
    memcpy(eth_hdr->payload, temp, pkt_size);
   // SET_COMMON_ETH_FCS(eth_hdr, pkt_size, 0);
    free(temp);
    return eth_hdr;
}