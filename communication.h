#ifndef __COMMUNICATION__
#define __COMMUNICATION__
#include "graph.h"

/*
*
* the api is used to send the packet to the neighbour node
* neighbour node which is connected through the link to the original node should receive the packet 
*
*
*/

int send_packet_out(char *pkt, unsigned int pkt_size, interface_t *original_intf);



#endif  /* __COMMUNICATION__ */