#include "communication.h"

static unsigned int udp_num = 40000;

static unsigned int get_udp_port() {
    return udp_num++;
}



void init_udp_socket(node_t *node) {
    if (node->udp_port_number) {
        return ; 
    }
    node->udp_port_number = get_udp_port();
    
}

int send_packet_out(char *pkt, unsigned int pkt_size, interface_t *original_intf) {

}