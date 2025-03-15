#include "layer2.h"
#include "tcpconst.h"
#include <arpa/inet.h>
#include <stdio.h>
#include "communication.h"  
#include "net.h"



void send_arp_broadcast_request(node_t *node, interface_t *oif, char *ip_addr)      {
    unsigned int payload_size = sizeof(arpheader_t);
    ethernetHeader_t *ethernet_header = (ethernetHeader_t *)calloc(1, ETH_HDR_SIZE_EXCL_PAYLOAD + payload_size);

    if (!oif) {
        oif = node_get_matching_subnet_interface(node,ip_addr);
        if(!oif){
            printf("Error : %s : No eligible subnet for ARP resolution for Ip-address : %s",
                    node->node_name, ip_addr);
            return;
        }
        if(strncmp(INTERFACE_IP(oif),ip_addr,16)==0) {
            printf("Error : %s : Attemp to resolve ARP for local Ip-address : %s",
                node->node_name, ip_addr);
        return;
        }
    }
    /* Prep of ethernet header*/
    layer2_add_broadcast_address(ethernet_header->dest.mac_address);
    memcpy(ethernet_header->src.mac_address, INTERFACE_MAC(oif), sizeof(mac_address_t));
    ethernet_header->type = ARP_MESSAGE;

    /* prepare broadcast request out message for interface*/
    arpheader_t *arp_hdr = (arpheader_t *)ethernet_header->payload;
    arp_hdr->hardware_type = 1;
    arp_hdr->protocol_type = 0x0800;
    arp_hdr->hardwareaddr_len = sizeof(mac_address_t);
    arp_hdr->protocoladdr_len = 4;
    arp_hdr->op_code = ARP_BROAD_REQ;

    memcpy(arp_hdr->sender_mac.mac_address,INTERFACE_MAC(oif),sizeof(mac_address_t));
    inet_pton(AF_INET, INTERFACE_IP(oif), &arp_hdr->src_ip);
    arp_hdr->src_ip = htonl(arp_hdr->src_ip);  

    memset(arp_hdr->destination_mac.mac_address,0,sizeof(mac_address_t));

    inet_pton(AF_INET, ip_addr, &arp_hdr->dest_ip);
    arp_hdr->dest_ip = htonl(arp_hdr->dest_ip);

    SET_COMMON_ETH_FCS(ethernet_header, sizeof(arpheader_t),0);

    send_packet_out((char *)ethernet_header, ETH_HDR_SIZE_EXCL_PAYLOAD + payload_size, oif);

    free(ethernet_header);

}

static void send_arp_reply_msg(ethernetHeader_t *ethernet_header, interface_t *oif) {
    printf("Debug: Sending ARP Reply from interface %s\n", oif->if_name);
    arpheader_t *arpheader = (arpheader_t *)(GET_ETHERNET_HEADER_PAYLOAD(ethernet_header));

    ethernetHeader_t *ethernetHdr_reply = (ethernetHeader_t *)calloc(1,MAX_PACKET_BUFFER_SIZE);

    memcpy(ethernetHdr_reply->dest.mac_address, arpheader->sender_mac.mac_address, sizeof(mac_address_t));
    memcpy(ethernetHdr_reply->src.mac_address, INTERFACE_MAC(oif), sizeof(mac_address_t));  

    ethernetHdr_reply->type = ARP_MESSAGE;

    arpheader_t *arp_header_reply = (arpheader_t *)(GET_ETHERNET_HEADER_PAYLOAD(ethernetHdr_reply));

    arp_header_reply->hardware_type = 1; 
    arp_header_reply->protocol_type = 0x0800;
    arp_header_reply->hardwareaddr_len = sizeof(mac_address_t);
    arp_header_reply->protocoladdr_len = 4;

    arp_header_reply->op_code = ARP_REPLY;  
    memcpy(arp_header_reply->sender_mac.mac_address, INTERFACE_MAC(oif), sizeof(mac_address_t));    

    inet_pton(AF_INET, INTERFACE_IP(oif), &arp_header_reply->src_ip);   
    arp_header_reply->src_ip = htonl(arp_header_reply->src_ip);

    memcpy(arp_header_reply->destination_mac.mac_address, arpheader->sender_mac.mac_address, sizeof(mac_address_t));     
    arp_header_reply->dest_ip = arpheader->src_ip;

    SET_COMMON_ETH_FCS(ethernetHdr_reply, sizeof(arpheader_t), 0); /* it is not used*/
     
    unsigned int total_pkt_size = ETH_HDR_SIZE_EXCL_PAYLOAD + sizeof(arpheader_t);
    char *shifted_pkt_buffer = pkt_buffer_shift_right((char *)ethernetHdr_reply, total_pkt_size, MAX_PACKET_BUFFER_SIZE);

    send_packet_out(shifted_pkt_buffer, total_pkt_size, oif);

    free(ethernetHdr_reply);
    

}


static void process_arp_broadcast_message_req(node_t *node, interface_t *iif, ethernetHeader_t *ethernet_hdr) {
    printf("%s : ARP Broadcast msg recvd on interface %s of node %s\n", 
        __FUNCTION__, iif->if_name , iif->att_node->node_name); 

    char ip_addr[16];
    arpheader_t *arp_hdr = (arpheader_t *)(GET_ETHERNET_HEADER_PAYLOAD(ethernet_hdr));
    unsigned int arp_dest_ip_addr = htonl(arp_hdr->dest_ip);
    inet_ntop(AF_INET, &arp_dest_ip_addr, ip_addr, 16);
    ip_addr[15] = '\0';
    
    if (strncmp(INTERFACE_IP(iif), ip_addr, 16) == 0) {
        printf("ARP Request for self IP address %s\n", ip_addr);
    
    }
    send_arp_reply_msg(ethernet_hdr,iif);
}




static void process_arp_reply_message(node_t *node, interface_t *iif, ethernetHeader_t *ethernet_hdr) {
    printf("%s : ARP reply msg recvd on interface %s of node %s\n",
        __FUNCTION__, iif->if_name , iif->att_node->node_name);

arp_table_update_from_arp_reply( NODE_ARP_TABLE(node), 
               (arpheader_t *)GET_ETHERNET_HEADER_PAYLOAD(ethernet_hdr), iif);   
}


void
init_arp_table(arp_table_t **arp_table){

    *arp_table = calloc(1, sizeof(arp_table_t));
    init_glthread(&((*arp_table)->arp_entries));
}


arp_entries_t * arp_table_entry_lookup(arp_table_t *arp_table, char *ip_addr) {
    glthread_t *curr;
    arp_entries_t *arp_entry;

    ITERATE_GLTHREAD_BEGIN(&arp_table->arp_entries,curr) {
        arp_entry = arp_glue_to_arp_entry(curr);
        if (strcmp(arp_entry->ip_address.ip_address, ip_addr) == 0) {
            return arp_entry;
        }
    } ITERATE_GLTHREAD_END(&arp_table->arp_entries,curr);
    return NULL ; 
}

void delete_arp_entry(arp_table_t *arp_table, char *ip_addr) {
    arp_entries_t *arp_entry = arp_table_entry_lookup(arp_table, ip_addr);
    if (!arp_entry) {
        return;
    }
    // add delete arp_entry(arp_entry);
}


bool_t arp_table_entry_addition(arp_table_t *arp_table, arp_entries_t *arp_entry) {
    arp_entries_t *arp_entry_old = arp_table_entry_lookup(arp_table,arp_entry->ip_address.ip_address);

    if (arp_entry_old && memcpy(arp_entry_old, arp_entry, sizeof(arp_entries_t)) == 0) {
        return FALSE;
    }

    if (arp_entry_old) {
        delete_arp_entry(arp_table, arp_entry->ip_address.ip_address);
    }
    init_glthread(&arp_entry->arp_glue);
    glthread_add_next(&arp_table->arp_entries, &arp_entry->arp_glue);
    return TRUE;
}




void arp_table_update_from_arp_reply(arp_table_t *arp_table, 
    arpheader_t *arp_hdr, interface_t *iif) {

unsigned int src_ip = 0;
assert(arp_hdr->op_code == ARP_REPLY);

arp_entries_t *arp_entry = calloc(1, sizeof(arp_entries_t));

// Fix: Convert network byte order to host byte order
src_ip = ntohl(arp_hdr->src_ip);
inet_ntop(AF_INET, &src_ip, arp_entry->ip_address.ip_address, 16);
arp_entry->ip_address.ip_address[15] = '\0';

memcpy(arp_entry->mac_address.mac_address, 
arp_hdr->sender_mac.mac_address, 
sizeof(mac_address_t));

strncpy(arp_entry->oif_name, iif->if_name, IF_NAME_SIZE);

bool_t rc = arp_table_entry_addition(arp_table, arp_entry);

if (rc == FALSE) {
printf("Error: Failed to add ARP entry for IP: %s\n", arp_entry->ip_address.ip_address);
free(arp_entry);
}
}

    

    void
    dump_arp_table(arp_table_t *arp_table){
    
        glthread_t *curr;
        arp_entries_t *arp_entry;
    
        ITERATE_GLTHREAD_BEGIN(&arp_table->arp_entries, curr){
    
            arp_entry = arp_glue_to_arp_entry(curr);
            
            printf("IP : %s, MAC : %u:%u:%u:%u:%u:%u, OIF = %s\n", 
                arp_entry->ip_address.ip_address, 
                arp_entry->mac_address.mac_address[0], 
                arp_entry->mac_address.mac_address[1], 
                arp_entry->mac_address.mac_address[2], 
                arp_entry->mac_address.mac_address[3], 
                arp_entry->mac_address.mac_address[4], 
                arp_entry->mac_address.mac_address[5], 
                arp_entry->oif_name);
            
        } ITERATE_GLTHREAD_END(&arp_table->arp_entries, curr);
    }



void layer2_frame_recv(node_t *node, interface_t *interface,
        char *pkt, unsigned int pkt_size) {
            ethernetHeader_t *ethernet_header = (ethernetHeader_t *)pkt;
            switch (ethernet_header ->type) {
                case ARP_MESSAGE:
                {
                    arpheader_t *arp_hdr = (arpheader_t *)ethernet_header->payload;
                    switch (arp_hdr->op_code) {
                        case ARP_BROAD_REQ:
                        process_arp_broadcast_message_req(node, interface, ethernet_header);
                        break;
                        case ARP_REPLY:
                        process_arp_reply_message(node, interface, ethernet_header);
                        break;
                        default:
                        break;
                    }
                }
                break;
                default:
                break;
                
            }
        }
    
void interface_set_l2_mode(node_t *node , interface_t *interface, char *l2_mode) {
    intf_l2_mode_t intf_l2_mode;
    
    if (strncmp(l2_mode,"access", strlen("access")) == 0) {
        intf_l2_mode = ACCESS;
    } else if (strncmp(l2_mode, "trunk", strlen("trunk")) == 0) {
        intf_l2_mode = TRUNK;
    } else {
        assert(0);
    }
}


void node_set_intf_l2_mode(node_t *node, char *intf_name, intf_l2_mode_t intf_l2_mode) {
    interface_t *interface = get_node_intf_by_name(node, intf_name);
    assert(interface);
    interface_set_l2_mode(node,interface, intf_l2_mode_str( intf_l2_mode));
}