#include "gluethread/glthread.h"
#include "net.h"
#include "graph.h"
#include <stdio.h>
#include <stdlib.h>
#include "layer2/layer2.h"
#include "communication.h"

typedef struct mac_table_entries {
    mac_address_t mac_address;
    char oif_name[IF_NAME_SIZE];
    glthread_t mac_entry_glue;
} mac_table_entries_t;




typedef struct mac_table_ {
    glthread_t mac_entries;
} mac_table_t;

GLTHREAD_TO_STRUCT(mac_entry_glue_to_mac_entry, mac_table_entries_t, mac_entry_glue);


void init_mac_table(mac_table_t **mac_table) {
    *mac_table = calloc(1, sizeof(mac_table_t));
    init_glthread(&(*mac_table)->mac_entries);
}

mac_table_entries_t *mac_table_entries_lookup(mac_table_t *mac_table, char *mac) {
    glthread_t *curr;
    mac_table_entries_t *mac_table_entry;

    ITERATE_GLTHREAD_BEGIN(&mac_table->mac_entries,curr) {
        mac_table_entry = mac_entry_glue_to_mac_entry(curr);
        if (strncmp(mac_table_entry->mac_address.mac_address, mac, sizeof(mac_address_t)) == 0) {
            return mac_table_entry;
        }
    } ITERATE_GLTHREAD_END(&mac_table->mac_entries,curr);
    return NULL;
}

void delete_mac_entry(mac_table_t *mac_table, char *mac) {
    mac_table_entries_t *mac_entry = mac_table_entries_lookup(mac_table,mac);
    if (!mac_entry) {
        return ; 
    }
    remove_glthread(&mac_entry->mac_entry_glue);
    free(mac_entry);
}


/*
macro
*/

#define IS_MAC_TABLE_ENTRY_EQUAL(mac_entry1, mac_entry2) \
    (strncmp(mac_entry1->mac_address.mac_address,mac_entry2->mac_address.mac_address, sizeof(mac_address_t)) == 0 && \
    strncmp(mac_entry1->oif_name, mac_entry2->oif_name, IF_NAME_SIZE) == 0)


bool_t add_mac_table_entry (mac_table_t *mac_table, mac_table_entries_t *mac_table_entries) {
    mac_table_entries_t *mac_table_entry_old = mac_table_entries_lookup(mac_table, mac_table_entries->mac_address.mac_address);
    if (mac_table_entry_old && IS_MAC_TABLE_ENTRY_EQUAL(mac_table_entry_old, mac_table_entries)) {
        return FALSE;
    }
    if (mac_table_entry_old) {
        delete_mac_entry(mac_table, mac_table_entries->mac_address.mac_address);
    }
    init_glthread(&mac_table_entries->mac_entry_glue);  
    glthread_add_next(&mac_table->mac_entries, &mac_table_entries->mac_entry_glue);
    return TRUE ; 
}



void dump_mac_table(mac_table_t *mac_table) {
    glthread_t *curr;
    mac_table_entries_t *mac_entry;

    ITERATE_GLTHREAD_BEGIN(&mac_table->mac_entries,curr) {
        mac_entry = mac_entry_glue_to_mac_entry(curr);
        printf("\tMAC : %u:%u:%u:%u:%u:%u   | Intf : %s\n", 
            mac_entry->mac_address.mac_address[0], 
            mac_entry->mac_address.mac_address[1],
            mac_entry->mac_address.mac_address[2],
            mac_entry->mac_address.mac_address[3], 
            mac_entry->mac_address.mac_address[4],
            mac_entry->mac_address.mac_address[5],
            mac_entry->oif_name);
    } ITERATE_GLTHREAD_END(&mac_table->mac_entries,curr);
}

static void l2_switch_perform_mac_learning(node_t *node, char *src_mac, char *if_name) {
    bool_t rc ;
    mac_table_entries_t *mac_table_entry = calloc(1, sizeof(mac_table_entries_t));
    memcpy(mac_table_entry->mac_address.mac_address,src_mac, sizeof(mac_address_t));    
    strncpy(mac_table_entry->oif_name, if_name, IF_NAME_SIZE);
    mac_table_entry->oif_name[IF_NAME_SIZE - 1] = '\0';
    rc  = add_mac_table_entry(NODE_MAC_TABLE(node), mac_table_entry);
    if (rc == FALSE) {
        free(mac_table_entry);
    }
}

// static void l2_switch_forward_frame(node_t *node, interface_t *interface, char *pkt, unsigned int pkt_size) {

//     /*if dst add is broadcast add then flood the frame*/
//     ethernetHeader_t *ethernet_header = (ethernetHeader_t *)pkt;
//     if (IS_MAC_BROADCAST_ADDR(ethernet_header->dest.mac_address)) {
//         send_pkt_flood(node, interface, pkt, pkt_size);
//         return ;
//     }

//     /*check mac table to forward the frame*/
//     mac_table_entries_t *mac_table_entry = 
//     mac_table_entries_lookup(NODE_MAC_TABLE(node), ethernet_header->dest.mac_address);

//     if (!mac_table_entry) {
//         send_pkt_flood(node, interface, pkt, pkt_size);
//         return ;
//     }
//     char *oif_name = mac_table_entry->oif_name;
//     interface_t *oif = get_node_if_by_name(node, oif_name);
//     if (!oif) {
//         return ;
//     }
//     send_packet_out(pkt, pkt_size, oif);
// }



static bool_t
l2_switch_send_pkt_out(char *pkt, unsigned int pkt_size,  interface_t *oif) {
    

    /* CASE 1 : If the interface is working int L3 mode then it should assert*/\

   assert(!IS_INTF_L3_MODE(oif));

    intf_l2_mode_t intf  = IF_L2_Mode(oif);
    if (intf == L2_MODE_UNKNOWN) {
        return FALSE ; 
    }

    ethernetHeader_t *ethernet_header = (ethernetHeader_t *)pkt;
    vlan_8021q_hdr_t *vlan_ethernet_hdr = is_pkt_vlan_tagged(ethernet_header);
    switch(intf) {
        case ACCESS :
        {
            unsigned int vlan_id = get_access_intf_operating_vlan_id(oif);
            /*
            CASE 1 : If the interface is in ACCESS mode but not in any vlan and the vlan id is also untagged then forward it 
            */
           if (!vlan_id && !vlan_ethernet_hdr) {
            send_packet_out(pkt, pkt_size, oif);
            return TRUE ; 
           }

           /*
           CASE 2 : If the interface is vlan aware but the packet is untagged just drop the packet 
           */
          if (vlan_id && !vlan_ethernet_hdr) {
            return FALSE ; 
          }

          /*
          CASE 3 : If the interface is vlan aware and the packet is tagged, then forward the frame only if the vlan id matches
          */
         if (vlan_ethernet_hdr && (GET_802_1Q_VLAN_ID(vlan_ethernet_hdr) == vlan_id)) {
            unsigned int new_pkt_size = 0 ; 
            ethernet_header = untag_pkt_with_vlan_id(ethernet_header, pkt_size, &new_pkt_size);
            send_packet_out((char *)ethernet_header, new_pkt_size, oif);
            return TRUE ;
         }

         /*
         CASE 4: If the interface not vlan aware and the packet is tagged with vlan then drop it
         */

         if (!vlan_id && vlan_ethernet_hdr) {
            return FALSE ; 
         }

        }
        break;

        case TRUNK: 

        /*CASE 5: If the packet is vlan tagged and the interface is aware of the vlan*/

        {
            unsigned int vlan_id = 0 ; 
            if (vlan_ethernet_hdr) {
                vlan_id = GET_802_1Q_VLAN_ID(vlan_ethernet_hdr);
            } 

            if (vlan_id && is_trunk_interface_vlan_enabled(oif,vlan_id)) {
                send_packet_out(pkt,pkt_size,oif);
                return TRUE ; 
            }
            /* do nothing */
            return FALSE ; 

        }
        break ; 
        case L2_MODE_UNKNOWN:
        break ; 
        default:
        ;
        return FALSE ; 
    }

}



static bool_t l2_switch_flood_pkt_out(node_t *node, interface_t *exempted_intf, char *pkt, unsigned int pkt_size) {

    interface_t *oif  = NULL ; 

    unsigned int i = 0 ; 

    char *pkt_copy = NULL ;
    char *temp_pkt = calloc(1, MAX_PACKET_BUFFER_SIZE);

    pkt_copy = temp_pkt + MAX_PACKET_BUFFER_SIZE - pkt_size ; 

    for ( ; i<MAX_INTF_PER_NODE; i++)  {
        oif = node->intf[i]; 
        if (!oif) {
            break ;
        }
        if (oif == exempted_intf || IS_INTF_L3_MODE(oif)){
            continue; /* skip the interface */
        }
        memcpy(pkt_copy, pkt, pkt_size);
      
        l2_switch_send_pkt_out(pkt_copy, pkt_size, oif);
    }

   free(temp_pkt);

}

               

static void
l2_switch_forward_frame(node_t *node, interface_t *recv_intf, 
                        ethernetHeader_t *ethernet_hdr, 
                        unsigned int pkt_size) {


    /*If dst mac is broadcast mac, then flood the frame*/
    if(IS_MAC_BROADCAST_ADDR(ethernet_hdr->dest.mac_address)){
        l2_switch_flood_pkt_out(node, recv_intf, (char *)ethernet_hdr, pkt_size);
        return;
    }

    /*Check the mac table to forward the frame*/
    mac_table_entries_t *mac_table_entry = 
        mac_table_entries_lookup(NODE_MAC_TABLE(node), ethernet_hdr->dest.mac_address);

    if(!mac_table_entry){
      
        l2_switch_flood_pkt_out(node, recv_intf, (char *)ethernet_hdr, pkt_size);
        return;
    }

    char *oif_name = mac_table_entry->oif_name;
   
    interface_t *oif = get_node_if_by_name(node, oif_name);

    if(!oif){
        return;
    }

 l2_switch_send_pkt_out((char *)ethernet_hdr, pkt_size, oif);
   
        
}

void layer2_switch_recv_frame(interface_t *interface, char *pkt, unsigned int pkt_size) {
 
    node_t *node = interface->att_node;
    ethernetHeader_t *ethernet_header = (ethernetHeader_t *)pkt;

    char *dst_mac = ethernet_header->dest.mac_address;
    char *src_add = ethernet_header->src.mac_address;

    l2_switch_perform_mac_learning(node,src_add,interface->if_name);
    l2_switch_forward_frame(node, interface, pkt, pkt_size);   

}

