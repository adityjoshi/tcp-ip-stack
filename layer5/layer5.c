#include "graph.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>





void layer5_ping(node_t *node,char *dst_ip_addr) {
    unsigned int addr_int ; 
    printf("Src node : %s, sending ping to %s\n", node->node_name, dst_ip_addr);

    inet_pton(AF_INET, dst_ip_addr, &addr_int);
    addr_int = htonl(addr_int);
    
}