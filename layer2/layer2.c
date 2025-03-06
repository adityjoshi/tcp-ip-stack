#include "layer2.h"
#include "tcpconst.h"
#include <arpa/inet.h>





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



void
arp_table_update_from_arp_reply(arp_table_t *arp_table, 
    arpheader_t *arp_hdr, interface_t *iif) {

        unsigned int src_ip = 0 ; 
        assert(arp_hdr->op_code == ARP_REPLY);
        
        arp_entries_t *arp_entry = calloc(1, sizeof(arp_entries_t));

        src_ip = htonl(arp_hdr->src_ip);

        inet_ntop(AF_INET, &src_ip, arp_entry->ip_address.ip_address, 16);
        arp_entry->ip_address.ip_address[15] = '\0';

        memcpy(arp_entry->mac_address.mac_address, arp_hdr->sender_mac.mac_address, sizeof(mac_address_t));

        strncpy(arp_entry->oif_name, iif->if_name, IF_NAME_SIZE);

        bool_t rc = arp_table_entry_addition(arp_table, arp_entry);

        if (rc == FALSE) {
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
    