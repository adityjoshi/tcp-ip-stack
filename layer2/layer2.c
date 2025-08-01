#include "layer2.h"
#include "layer3/layer3.h"
#include "tcpconst.h"
#include <arpa/inet.h>
#include <stdio.h>
#include "communication.h"  
#include "net.h"
#include <assert.h>



extern bool_t
is_layer3_local_delivery(node_t *node, 
                         uint32_t dst_ip);

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

     
    if(strncmp(INTERFACE_IP(iif), ip_addr, 16) != 0){
        
        printf("%s : ARP Broadcast req msg dropped, Dst IP address %s did not match with interface ip : %s\n", 
                node->node_name, ip_addr , INTERFACE_IP(iif));
        return;
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

    if (arp_entry_old && memcmp(arp_entry_old, arp_entry, sizeof(arp_entries_t)) == 0) {
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



static void 
pending_arp_processing_callback_function(node_t *node,
                                         interface_t *oif,
                                         arp_entries_t *arp_entry,
                                         arp_pending_entry_t *arp_pending_entry) {

ethernetHeader_t *ethernetHeader = (ethernetHeader_t *)arp_pending_entry->pkt; 
unsigned int pkt_size = arp_pending_entry->pkt_size;
memcpy(ethernetHeader->dest.mac_address, arp_entry->mac_address.mac_address, sizeof(mac_address_t));
memcpy(ethernetHeader->src.mac_address, INTERFACE_MAC(oif), sizeof(mac_address_t));
SET_COMMON_ETH_FCS(ethernetHeader, pkt_size - GET_ETH_HDR_SIZE_EXCL_PAYLOAD(ethernetHeader), 0);
send_packet_out((char *)ethernetHeader,pkt_size,oif);

};
    

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

    extern void layer2_switch_recv_frame(interface_t *interface, char *pkt, unsigned int pkt_size);


// void layer2_frame_recv(node_t *node, interface_t *interface,
//         char *pkt, unsigned int pkt_size) {
//             ethernetHeader_t *ethernet_header = (ethernetHeader_t *)pkt;
//             switch (ethernet_header ->type) {
//                 case ARP_MESSAGE:
//                 {
//                     arpheader_t *arp_hdr = (arpheader_t *)ethernet_header->payload;
//                     switch (arp_hdr->op_code) {
//                         case ARP_BROAD_REQ:
//                         process_arp_broadcast_message_req(node, interface, ethernet_header);
//                         break;
//                         case ARP_REPLY:
//                         process_arp_reply_message(node, interface, ethernet_header);
//                         break;
//                         default:
//                         break;
//                     }
//                 }
//                 break;
//                 default:
//                 break;
                
//             }
//         }


extern void promote_pkt_to_layer3(node_t *node, interface_t *interface,
                         char *pkt, unsigned int pkt_size,
                         int L3_protocol_type) ; 

void
promote_pkt_to_layer2(node_t *node, interface_t *iif,
        ethernetHeader_t *ethernet_hdr,
        uint32_t pkt_size){
  switch (ethernet_hdr->type) {
                        case ARP_MESSAGE:
                        {
                            arpheader_t *arp_hdr = (arpheader_t *)ethernet_hdr->payload;
                            switch (arp_hdr->op_code) {
                                case ARP_BROAD_REQ:
                                process_arp_broadcast_message_req(node, iif, ethernet_hdr);
                                break;
                                case ARP_REPLY:
                                process_arp_reply_message(node, iif, ethernet_hdr);
                                break;
                                default:
                                break;
                            }
                        }
                        break;
                         case ETH_IP:
                    promote_pkt_to_layer3(node, iif,
                    GET_ETHERNET_HDR_PAYLOAD(ethernet_hdr),
                    pkt_size - GET_ETH_HDR_SIZE_EXCL_PAYLOAD(ethernet_hdr),
                    ethernet_hdr->type);
                    break;
                        default:
                        ;
                    }



                }

        void layer2_frame_recv(node_t *node, interface_t *interface,
            char *pkt, unsigned int pkt_size) {



                unsigned int vlan_id_to_tag = 0 ; 
                ethernetHeader_t *ethernet_header = (ethernetHeader_t *)pkt;
                printf(interface, ethernet_header);
                if (l2_frame_recv_qualify_on_interface(interface,ethernet_header,&vlan_id_to_tag) == FALSE) {
                    printf("L2 frame has been rejected");
                    return ;
                }
                printf("L2 frame has been accepted\n");

                if (IS_INTF_L3_MODE(interface)) {
                   promote_pkt_to_layer2(node,interface, ethernet_header, pkt_size);
                 
                  
                }

                else if (IF_L2_Mode(interface) == ACCESS || IF_L2_Mode(interface) == TRUNK ) {
                      unsigned int new_pkt_size = 0;

        if(vlan_id_to_tag){
            pkt = (char *)tag_pkt_with_vlan_id((ethernetHeader_t *)pkt,
                                                pkt_size, vlan_id_to_tag,
                                                &new_pkt_size);
            assert(new_pkt_size != pkt_size);
        }
                    layer2_switch_recv_frame(interface, pkt, vlan_id_to_tag ? new_pkt_size : pkt_size);
                } else {
                    return ; /*do nothing, drop the packet*/
                }


            }
        


void add_arp_pending_entry(arp_entries_t *arp_entry, arp_proceesing_func cb, char *pkt, unsigned int pkt_size) {
      arp_pending_entry_t *arp_pending_entry = 
        calloc(1, sizeof(arp_pending_entry_t) + pkt_size);

    init_glthread(&arp_pending_entry->arp_pending_glue);
    arp_pending_entry->cb = cb;
    arp_pending_entry->pkt_size = pkt_size;
    memcpy(arp_pending_entry->pkt, pkt, pkt_size);

    glthread_add_next(&arp_entry->arp_pending_list, 
                    &arp_pending_entry->arp_pending_glue);
}


void
create_arp_sane_entry(arp_table_t *arp_table, char *ip_addr){

    /*case 1 : If full entry already exist - assert. The L2 must have
     * not create ARP sane entry if the already was already existing*/

    arp_entries_t *arp_entry = arp_table_entry_lookup(arp_table, ip_addr);
    
    if(arp_entry){
    
        if(!arp_entry_sane(arp_entry)){
            assert(0);
        }

       
        return arp_entry;
    }

    /*if ARP entry do not exist, create a new sane entry*/
    arp_entry = calloc(1, sizeof(arp_entries_t));
    stpncpy(arp_entry->ip_address.ip_address, ip_addr, 16);
    arp_entry->ip_address.ip_address[15] = '\0';
    init_glthread(&arp_entry->arp_pending_list);
    arp_entry->is_sane = TRUE;
    bool_t rc = arp_table_entry_add(arp_table, arp_entry, 0);
    if(rc == FALSE){
        assert(0);
    }
}



static void l2_forward_ip_packet(node_t *node, unsigned int next_hop_ip, char *outgoing_if, ethernetHeader_t *pkt, unsigned int pkt_size)  {
    interface_t *oif = NULL ; 
    char next_hop_ip_str[16];
    arp_entries_t *arp_entry = NULL;
    ethernetHeader_t *ethernet_header = (ethernetHeader_t *)pkt;
    unsigned int ethernet_payload_size = pkt_size - ETH_HDR_SIZE_EXCL_PAYLOAD;

    next_hop_ip  = htonl(next_hop_ip);
    inet_ntop(AF_INET, &next_hop_ip, next_hop_ip_str, 16);
    next_hop_ip = htonl(next_hop_ip);

    if (outgoing_if) {

        /*
        
        case 1 : it means we have a next hop and l2 will forward the packet to the next hops
        
        */

        oif = get_node_if_by_name(node, outgoing_if);
        assert(oif);

        arp_entry = arp_table_entry_lookup(NODE_ARP_TABLE(node), next_hop_ip_str);


        if (!arp_entry)  {
            /*
            if the entry is not present in the arp table then it means its the time for the arp broadcast request 
            */
        //    assert(0);
        //    send_arp_broadcast_request(node, oif, next_hop_ip_str);
        //    return ; 

        create_arp_sane_entry(NODE_ARP_TABLE(node),next_hop_ip_str);
        add_arp_pending_entry(arp_entry, pending_arp_processing_callback_function,(char *)pkt,pkt_size);
        send_arp_broadcast_request(node, oif, next_hop_ip_str);
        return ;


        } else if (arp_enty_sane(arp_entry)) {
             add_arp_pending_entry(arp_entry, pending_arp_processing_callback_function,(char *)pkt,pkt_size);
             return;
        }
        goto l2_frame_prepare;
    }

    /*
    case 4 : self ping 
    */

    if (is_layer3_local_delivery(node, next_hop_ip)) {
        promote_pkt_to_layer3(node,0, GET_ETHERNET_HDR_PAYLOAD(ethernet_header), pkt_size - GET_ETH_HDR_SIZE_EXCL_PAYLOAD(ethernet_header), ethernet_header->type);
        return;
    }

    /*
    
    case 2: if it is the direct route basically it is present in 
    the local subnet so we can send the packet to the interface

    */

    oif = node_get_matching_subnet_interface(node, next_hop_ip_str);
    if (!oif) {
       printf("%s : Error : Local matching subnet for IP : %s could not be found\n",
                    node->node_name, next_hop_ip_str);
        return;
    }

    arp_entry = arp_table_entry_lookup(NODE_ARP_TABLE(node), next_hop_ip_str);

    if (!arp_entry)  {
        /*
        if the entry is not present in the arp table then it means its the time for the arp broadcast request 
        */
       assert(0);
       send_arp_broadcast_request(node, oif, next_hop_ip_str);
       return ; 
    }

    l2_frame_prepare:
    memcpy(ethernet_header->dest.mac_address, arp_entry->mac_address.mac_address, sizeof(mac_address_t));
    memcpy(ethernet_header->src.mac_address, INTERFACE_MAC(oif), sizeof(mac_address_t));
    SET_COMMON_ETH_FCS(ethernet_header, ethernet_payload_size, 0);
    send_packet_out((char *)ethernet_header, pkt_size, oif);



}

static void layer2_pkt_recv_from_top(node_t *node, unsigned int next_hop_ip, char *outgoing_if, char *pkt, unsigned int pkt_size, int protocol_number)  {

    assert(pkt_size < sizeof(((ethernetHeader_t *)0)->payload)) ;

    if (protocol_number == ETH_IP) {
        ethernetHeader_t *ethernet_hdr_empty = ALLOC_ETH_HDR_WITH_PAYLOAD(pkt,pkt_size);
        ethernet_hdr_empty->type = ETH_IP;

        l2_forward_ip_packet(node,next_hop_ip,outgoing_if,ethernet_hdr_empty, pkt_size+ETH_HDR_SIZE_EXCL_PAYLOAD);
    }

}



void demote_pkt_layer2(node_t *node, unsigned int next_hop_ip, char *outgoing_intf, char *pkt, unsigned int pkt_size, int protocol_number){ 
 layer2_pkt_recv_from_top(node, next_hop_ip, outgoing_intf, pkt, pkt_size, protocol_number);
}



void interface_set_vlan(node_t *node, interface_t *interface, unsigned int vlan_id) {
    
    /*
    Case 1: if the interface has ip then can't set 
    */
   if (IS_INTF_L3_MODE(interface)) {
    printf("Error : Interface %s : L3 mode enabled\n", interface->if_name);
        return;
   }

   /*
   Case 2 : can't set the vlan if the interface is not in l2 mode 
   */

   if (IF_L2_Mode(interface) != ACCESS && IF_L2_Mode(interface) != TRUNK) {
    printf("Error : Interface %s : L2 mode not set\n", interface->if_name);
        return;
   }

   /*
   Case 3 :  Can set only one vlan on interface operating in ACCESS mode
   */

    if(interface->interface_nw_props.intf_l2_mode == ACCESS){
        
        unsigned int i = 0, *vlan = NULL;    
        for( ; i < MAX_VLAN_MEMBERSHIP; i++){
            if(interface->interface_nw_props.vlans[i]){
                vlan = &interface->interface_nw_props.vlans[i];
            }
        }
        if(vlan){
            *vlan = vlan_id;
            return;
        }
        interface->interface_nw_props.vlans[0] = vlan_id;
    }
    /*case 4 : Add vlan membership on interface operating in TRUNK mode*/
    if(interface->interface_nw_props.intf_l2_mode == TRUNK){

        unsigned int i = 0, *vlan = NULL;

        for( ; i < MAX_VLAN_MEMBERSHIP; i++){

            if(!vlan && interface->interface_nw_props.vlans[i] == 0){
                vlan = &interface->interface_nw_props.vlans[i];
            }
            else if(interface->interface_nw_props.vlans[i] == vlan_id){
                return;
            }
        }
        if(vlan){
            *vlan = vlan_id;
            return;
        }
        printf("Error : Interface %s : Max Vlan membership limit reached", interface->if_name);
    }
}
void
interface_unset_l2_mode(node_t *node, 
                      interface_t *interface, 
                      char *l2_mode_option){

    
}


void
interface_unset_vlan(node_t *node,
                   interface_t *interface,
                   unsigned int vlan){

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

    /*
    Case 1 : If the interface is in L3 mode, then we need to set it to L2 mode.
    So disable the ip and set it to the l2 mode 
    */

    if(IS_INTF_L3_MODE(interface)) {
        interface->interface_nw_props.is_ipadd_config_backup = TRUE ; 
        interface->interface_nw_props.is_ip_address_config = FALSE ; 
        IF_L2_Mode(interface) = intf_l2_mode;
        return ; 
    }

    /*
    Case 2 : If the interface is neither working l2 mode or it is working in l3 mode.
    */

    if (IF_L2_Mode(interface) ==  L2_MODE_UNKNOWN) {
        IF_L2_Mode(interface) = intf_l2_mode;
    }


    /*
    Case 3: if the user operating in the same mode that the user entered then do nothign
    */
   if (IF_L2_Mode(interface) == intf_l2_mode) {
    return ;
}

/*
Case 4: if the interface is working in the access mode and the user want in the trunk mode then overwrite 
*/

if (IF_L2_Mode(interface) == ACCESS && intf_l2_mode == TRUNK) {
    IF_L2_Mode(interface) = TRUNK ; 
    return ; 
}


/*
Case 5: If the interface is working in the trunk mode and the user want access mode then overwrite, 
remove all vlans from interface, user must enable vlan again  on interface
*/

if (IF_L2_Mode(interface) == TRUNK && intf_l2_mode == ACCESS) {

    IF_L2_Mode(interface) = ACCESS; 
   
    unsigned int i = 0 ; 
    for (i ; i<MAX_VLAN_MEMBERSHIP; i++) {
        interface->interface_nw_props.vlans[i] = 0 ;  
    }
}

}




void node_set_intf_l2_mode(node_t *node, char *intf_name, intf_l2_mode_t intf_l2_node) {
    interface_t *interface = get_node_if_by_name(node, intf_name);
    assert(interface);
    interface_set_l2_mode(node, interface, intf_l2_mode_str( intf_l2_node));
}


void node_set_intf_vlan_membership(node_t *node, char *intf_name, unsigned int vlan_id) {
    interface_t *interface = get_node_if_by_name(node, intf_name);
    assert(interface);
  interface_set_vlan(node, interface, vlan_id);

}



/*

VLAN MANAGEMENT

*/


/*RETURN NEW PACKET SIZE IF THE VLAN IS TAGGED*/

// ethernetHeader_t *tag_pkt_with_vlan_id(ethernetHeader_t *ethernet_hdr, unsigned int total_pkt_size, int vlan_id,  unsigned int *new_pkt_size) {
//     unsigned int payload_size = 0 ;
//     *new_pkt_size = 0;

//          /*If the pkt is already tagged, replace it*/

//     vlan_8021q_hdr_t *vlan_8021q_hdr = is_pkt_vlan_tagged(ethernet_hdr);

//     if (vlan_8021q_hdr) {
//         payload_size = total_pkt_size - VLAN_ETH_HDR_SIZE_EXCL_PAYLOAD;
//         vlan_8021q_hdr->tci_vid = (short)vlan_id;
//         SET_COMMON_ETH_FCS(ethernet_hdr, payload_size, 0);
//         *new_pkt_size = total_pkt_size;  
//         return ethernet_hdr;
//     }


//     ethernetHeader_t *ethernet_hdr_old;
//     memcpy((char *)&ethernet_hdr_old, (char *)ethernet_hdr,ETH_HDR_SIZE_EXCL_PAYLOAD - sizeof(ethernet_hdr_old->FCS));

//     payload_size = total_pkt_size - ETH_HDR_SIZE_EXCL_PAYLOAD;

//     vlan_ethernet_hdr_t *vlan_ethernet_hdr =   (vlan_ethernet_hdr_t *)((char *)ethernet_hdr - sizeof(vlan_8021q_hdr_t));

//     memset((char *)vlan_ethernet_hdr,0,VLAN_ETH_HDR_SIZE_EXCL_PAYLOAD-sizeof(vlan_ethernet_hdr->FCS));
//     memcpy(vlan_ethernet_hdr->dst_mac.mac_address,ethernet_hdr_old->dest.mac_address,sizeof(mac_address_t));
//     memcpy(vlan_ethernet_hdr->src_mac.mac_address, ethernet_hdr_old->src.mac_address,sizeof(mac_address_t));

//     /*802.1Q vlan hdr*/

//     vlan_ethernet_hdr->vlan_8021q_hdr.tpid = VLAN_8021Q_PROTO;
//     vlan_ethernet_hdr->vlan_8021q_hdr.tci_pcp = 0;
//     vlan_ethernet_hdr->vlan_8021q_hdr.tci_dei = 0;
//     vlan_ethernet_hdr->vlan_8021q_hdr.tci_vid = (short)vlan_id;

//     vlan_ethernet_hdr->type = ethernet_hdr_old->type;

//      SET_COMMON_ETH_FCS((ethernetHeader_t *)vlan_ethernet_hdr, payload_size, 0 );
//      *new_pkt_size = VLAN_ETH_HDR_SIZE_EXCL_PAYLOAD + payload_size;
//      return (ethernetHeader_t *)vlan_ethernet_hdr;
// }


ethernetHeader_t *tag_pkt_with_vlan_id(ethernetHeader_t *ethernet_hdr, unsigned int total_pkt_size, int vlan_id, unsigned int *new_pkt_size) {
    *new_pkt_size = 0;

    // Check if packet is already VLAN-tagged
    vlan_8021q_hdr_t *vlan_8021q_hdr = is_pkt_vlan_tagged(ethernet_hdr);
    if (vlan_8021q_hdr) {
        unsigned int payload_size = total_pkt_size - VLAN_ETH_HDR_SIZE_EXCL_PAYLOAD;
        vlan_8021q_hdr->tci_vid = (short)vlan_id;
        SET_COMMON_ETH_FCS(ethernet_hdr, payload_size, 0);
        *new_pkt_size = total_pkt_size;
        return ethernet_hdr;
    }

    // Allocate space for the new VLAN-tagged header
    vlan_ethernet_hdr_t *vlan_ethernet_hdr = (vlan_ethernet_hdr_t *)malloc(VLAN_ETH_HDR_SIZE_EXCL_PAYLOAD + (total_pkt_size - ETH_HDR_SIZE_EXCL_PAYLOAD));
    if (!vlan_ethernet_hdr) {
        fprintf(stderr, "Failed to allocate memory for VLAN tag\n");
        return NULL;
    }

    // Copy old Ethernet header (excluding FCS)
    memcpy(vlan_ethernet_hdr->dst_mac.mac_address, ethernet_hdr->dest.mac_address, sizeof(mac_address_t));
    memcpy(vlan_ethernet_hdr->src_mac.mac_address, ethernet_hdr->src.mac_address, sizeof(mac_address_t));

    // Set VLAN fields
    vlan_ethernet_hdr->vlan_8021q_hdr.tpid = VLAN_8021Q_PROTO;
    vlan_ethernet_hdr->vlan_8021q_hdr.tci_pcp = 0;
    vlan_ethernet_hdr->vlan_8021q_hdr.tci_dei = 0;
    vlan_ethernet_hdr->vlan_8021q_hdr.tci_vid = (short)vlan_id;
    vlan_ethernet_hdr->type = ethernet_hdr->type;

    // Copy payload (if any)
    unsigned int payload_size = total_pkt_size - ETH_HDR_SIZE_EXCL_PAYLOAD;
    if (payload_size > 0) {
        char *payload_src = (char *)ethernet_hdr + ETH_HDR_SIZE_EXCL_PAYLOAD;
        char *payload_dst = (char *)vlan_ethernet_hdr + VLAN_ETH_HDR_SIZE_EXCL_PAYLOAD;
        memcpy(payload_dst, payload_src, payload_size);
    }

    SET_COMMON_ETH_FCS((ethernetHeader_t *)vlan_ethernet_hdr, payload_size, 0);
    *new_pkt_size = VLAN_ETH_HDR_SIZE_EXCL_PAYLOAD + payload_size;
    return (ethernetHeader_t *)vlan_ethernet_hdr;
}

ethernetHeader_t *untag_pkt_with_vlan_id(ethernetHeader_t *ethernet_hdr, unsigned int total_pkt_size,  unsigned int *new_pkt_size) {
    *new_pkt_size = 0;

    vlan_8021q_hdr_t *vlan_8021q_hdr = is_pkt_vlan_tagged(ethernet_hdr);

    if (!vlan_8021q_hdr) {
        *new_pkt_size = total_pkt_size;
        return ethernet_hdr;
    }

    vlan_ethernet_hdr_t vlan_ethernet_hdr_old;
    memcpy((char *)&vlan_ethernet_hdr_old,(char *)ethernet_hdr, VLAN_ETH_HDR_SIZE_EXCL_PAYLOAD - sizeof(vlan_ethernet_hdr_old.FCS));

    ethernet_hdr = (ethernetHeader_t *)((char *)ethernet_hdr + sizeof(vlan_8021q_hdr_t));
    memcpy(ethernet_hdr->dest.mac_address, vlan_ethernet_hdr_old.dst_mac.mac_address, sizeof(mac_address_t));
    memcpy(ethernet_hdr->src.mac_address, vlan_ethernet_hdr_old.src_mac.mac_address, sizeof(mac_address_t));
    ethernet_hdr->type = vlan_ethernet_hdr_old.type;
    unsigned int payload_size = total_pkt_size - VLAN_ETH_HDR_SIZE_EXCL_PAYLOAD;    
      SET_COMMON_ETH_FCS(ethernet_hdr, payload_size, 0);
    
    *new_pkt_size = total_pkt_size - sizeof(vlan_8021q_hdr_t);
    return ethernet_hdr;
}
                     


void
demote_pkt_to_layer2(node_t *node, /*Currenot node*/ 
        unsigned int next_hop_ip,  /*If pkt is forwarded to next router, then this is Nexthop IP address (gateway) provided by L3 layer. L2 need to resolve ARP for this IP address*/
        char *outgoing_intf,       /*The oif obtained from L3 lookup if L3 has decided to forward the pkt. If NULL, then L2 will find the appropriate interface*/
        char *pkt, unsigned int pkt_size,   /*Higher Layers payload*/
        int protocol_number){               /*Higher Layer need to tell L2 what value need to be feed in eth_hdr->type field*/

    layer2_pkt_recv_from_top(node, next_hop_ip,
        outgoing_intf, pkt, pkt_size,
        protocol_number);
}