#include "communication.h"
#include "graph.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread/pthread.h>


static char recv_buffer[MAX_PACKET_BUFFER_SIZE];
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


static void _pkt_receive(node_t *receiver_node,char *pkt_with_auxillary_data, unsigned int pkt_size) {
    char *receiver_interface_name = pkt_with_auxillary_data;
    interface_t *recv_intf = get_node_if_by_name(receiver_node,receiver_interface_name);

    if(!recv_intf){
        printf("Error : Pkt recvd on unknown interface %s on node %s\n", 
                    recv_intf->if_name, receiver_node->node_name);
        return;
    }
    pkt_receive(receiver_node, recv_intf, pkt_with_auxillary_data + IF_NAME_SIZE, 
        pkt_size - IF_NAME_SIZE);
}


static void *  _network_start_pkt_receiver_thread(void *arg) {
    node_t *node ; 
    glthread_t *curr;

    fd_set active_fd_set, back_fd_set;
    int sock_max_fd  = 0 ;
    int bytes_recvd = 0;
    
    graph_t *topo = (void *)arg;
    int addr_len = sizeof(struct sockaddr);

    FD_ZERO(&active_fd_set);
    FD_ZERO(&back_fd_set);

     
    struct sockaddr_in sender_addr;

    ITERATE_GLTHREAD_BEGIN(&topo->node_list, curr){

        node = graph_glue_to_node(curr);
        
        if(!node->udp_socket_fd) 
            continue;

        if(node->udp_socket_fd > sock_max_fd)
            sock_max_fd = node->udp_socket_fd;

        FD_SET(node->udp_socket_fd, &back_fd_set);
            
    } ITERATE_GLTHREAD_END(&topo->node_list, curr);
    while(1) {
        memcpy(&active_fd_set, &back_fd_set, sizeof(fd_set));
        select(sock_max_fd+1, &active_fd_set, NULL, NULL, NULL);
        ITERATE_GLTHREAD_BEGIN(&topo->node_list,curr) {
            node = graph_glue_to_node(curr);
            /*
            * here we are first resetting the buffer to 0 so that no garbage value is present in the buffer
            * then bytes_recvd is used to store the number of bytes received from the sender
            * _pkt_receive is used to receive the packet and process it
            * 
            */
            memset(recv_buffer, 0, MAX_PACKET_BUFFER_SIZE);
            bytes_recvd = recvfrom(node->udp_socket_fd, (char *)recv_buffer, 
                    MAX_PACKET_BUFFER_SIZE, 0, (struct sockaddr *)&sender_addr, &addr_len);
            
            _pkt_receive(node, recv_buffer, bytes_recvd);
        }

    } ITERATE_GLTHREAD_END(&topo->node_list, curr);
}

void network_start_packet_receiver_thread(graph_t *topo) {
    pthread_attr_t attr;
    pthread_t recv_packet_thread;

    pthread_attr_t_init(&attr);
    pthread_attr_t_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    pthread_create(&recv_packet_thread, &attr, 
        _network_start_pkt_receiver_thread, 
        (void *)topo);
}





/*
*
* PUBLIC APIs 
* 
*/

int send_packet_out(char *pkt, unsigned int pkt_size, interface_t *original_intf) {
    
}
