#include "graph.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>




extern void
demote_pkt_to_layer3(node_t *node,
        char *pkt, unsigned int size,
        int protocol_number,
        unsigned int dest_ip_address);

void layer5_ping(node_t *node,char *dst_ip_addr) {
    unsigned int addr_int ; 
    printf("Src node : %s, sending ping to %s\n", node->node_name, dst_ip_addr);

    inet_pton(AF_INET, dst_ip_addr, &addr_int);
    addr_int = htonl(addr_int);
    demote_pkt_to_layer3(node, NULL, 0, ICMP_PRO, addr_int);
}