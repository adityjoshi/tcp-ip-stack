#ifndef __COMMUNICATION__
#define __COMMUNICATION__
#include "graph.h"


#define MAX_PACKET_BUFFER_SIZE 2048




/*
*
* the api is used to send the packet to the neighbour node
* neighbour node which is connected through the link to the original node should receive the packet 
*
*
*/

int send_packet_out(char *pkt, unsigned int pkt_size, interface_t *original_intf);

/*API to recv packet from interface*/
int
pkt_receive(node_t *node, interface_t *interface, 
            char *pkt, unsigned int pkt_size);

#endif  /* __COMMUNICATION__ */