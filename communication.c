#include "communication.h"
#include "graph.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread/pthread.h>
#include <errno.h>
#include <Kernel/string.h>
#include <netdb.h>
#include <stdio.h>
#include <sys/select.h> 
#include <unistd.h>


static char recv_buffer[MAX_PACKET_BUFFER_SIZE];
static char send_buffer[MAX_PACKET_BUFFER_SIZE];
static unsigned int udp_num = 40000;

static unsigned int get_udp_port() {
    return udp_num++;
}

static int _send_pkt_out(int sock_fd, char *pkt_data, unsigned int pkt_size,  unsigned int dst_udp_port_no) {
        int rc ; 
        struct sockaddr_in dest_addr;
   
        struct hostent *host = (struct hostent *) gethostbyname("127.0.0.1"); 
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = dst_udp_port_no;
        dest_addr.sin_addr = *((struct in_addr *)host->h_addr);
    
        rc = sendto(sock_fd, pkt_data, pkt_size, 0, 
                (struct sockaddr *)&dest_addr, sizeof(struct sockaddr));
      
        
        return rc;
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
        printf("Error : Pkt recvd on unknown interface on node %s\n", receiver_node->node_name);

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
           if(FD_ISSET(node->udp_socket_fd, &active_fd_set)){
    
            memset(recv_buffer, 0, MAX_PACKET_BUFFER_SIZE);
            bytes_recvd = recvfrom(node->udp_socket_fd, (char *)recv_buffer, 
                    MAX_PACKET_BUFFER_SIZE, 0, (struct sockaddr *)&sender_addr, &addr_len);
            
            _pkt_receive(node, recv_buffer, bytes_recvd);
        }
        }

    } ITERATE_GLTHREAD_END(&topo->node_list, curr);
}

void network_start_packet_receiver_thread(graph_t *topo) {
    pthread_attr_t attr;
    pthread_t recv_packet_thread;

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    pthread_create(&recv_packet_thread, &attr, 
        _network_start_pkt_receiver_thread, 
        (void *)topo);
}





/*
*
* PUBLIC APIs 
* 
*/
int
send_packet_out(char *pkt, unsigned int pkt_size, 
             interface_t *interface){

    int rc = 0;

    node_t *sending_node = interface->att_node;
    node_t *nbr_node = get_nbr_node(interface);
    
    if(!nbr_node)
        return -1;

    if(pkt_size + IF_NAME_SIZE > MAX_PACKET_BUFFER_SIZE){
        printf("Error : Node :%s, Pkt Size exceeded\n", sending_node->node_name);
        return -1;
    }

    unsigned int dst_udp_port_no = nbr_node->udp_port_number;
    
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP );

    if(sock < 0){
        printf("Error : Sending socket Creation failed , errno = %d", errno);
        return -1;
    }
    
    interface_t *other_interface = &interface->link->intf1 == interface ? \
                                    &interface->link->intf2 : &interface->link->intf1;

    memset(send_buffer, 0, MAX_PACKET_BUFFER_SIZE);

    char *pkt_with_aux_data = send_buffer;

    strncpy(pkt_with_aux_data, other_interface->if_name, IF_NAME_SIZE);

    pkt_with_aux_data[IF_NAME_SIZE - 1] = '\0';

    memcpy(pkt_with_aux_data + IF_NAME_SIZE, pkt, pkt_size);

    rc = _send_pkt_out(sock, pkt_with_aux_data, pkt_size + IF_NAME_SIZE, 
                        dst_udp_port_no);

    close(sock);
    return rc; 
}

extern void
layer2_frame_recv(node_t *node, interface_t *interface,
                     char *pkt, unsigned int pkt_size);

int
pkt_receive(node_t *node, interface_t *interface,
            char *pkt, unsigned int pkt_size){
             
                 printf("[DEBUG] pkt_receive called on node %s interface %s, pkt_size: %u\n", 
           node->node_name, interface->if_name, pkt_size);
    
    /*
    make room in the packet buffer shift the data to the right so that the tcp/ipc can append more
    headers in the packet without any issye 
    
    */
                pkt = pkt_buffer_shift_right(pkt,pkt_size,MAX_PACKET_BUFFER_SIZE - IF_NAME_SIZE);

                 /*Do further processing of the pkt here*/
                layer2_frame_recv(node, interface, pkt, pkt_size );
                return 0 ; 

            }

/*Packet flooding across all the interface*/

int send_pkt_flood(node_t *node, interface_t *exempted_intf,char *pkt, unsigned int pkt_size) {
    unsigned int i = 0 ; 
    interface_t *intf = NULL;
    for (i ; i < MAX_INTF_PER_NODE; i++) {
        intf = node->intf[i];
        if (!intf) {
            return 0; 
        }
        if (intf == exempted_intf) {
            continue;
        }
        send_packet_out(pkt, pkt_size, intf);
    }
    return 0;
}