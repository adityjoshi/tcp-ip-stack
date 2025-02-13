#include "communication.h"
#include "graph.h"
#include <sys/socket.h>
#include <netinet/in.h>

static unsigned int udp_num = 40000;

static unsigned int get_udp_port() {
    return udp_num++;
}



void init_udp_socket(node_t *node) {
    if (node->udp_port_number) {
        return ; 
    }
    node->udp_port_number = get_udp_port();
    int udp_socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_socket_fd == -1) {
        printf("Socket Creation Failed for node %s\n", node->node_name);
        return;   
    }

    struct sockaddr_in node_addr;
    node_addr.sin_family      = AF_INET;
    node_addr.sin_port        = node->udp_port_number;
    node_addr.sin_addr.s_addr = INADDR_ANY;
    if (bind(udp_socket_fd, (struct sockaddr *)&node_addr, sizeof(struct sockaddr)) == -1) {
        printf("Error : socket bind failed for Node %s\n", node->node_name);
        return;
    }

    node->udp_socket_fd = udp_socket_fd;
    return ;
}

int send_packet_out(char *pkt, unsigned int pkt_size, interface_t *original_intf) {

}