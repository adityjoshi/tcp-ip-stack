#include "layer3/layer3.h"

#include "arpa/inet.h"
#include <stdio.h>      
#include <stdlib.h>     
#include <string.h>     
#include <assert.h> 
#include "net.h"   
#include "graph.h"
#include "communication.h":
#include <assert.h>


extern void
demote_pkt_to_layer2(node_t *node,
                     unsigned int next_hop_ip,
                     char *outgoing_intf, 
                     char *pkt, unsigned int pkt_size,
                     int protocol_number);


static bool_t l3_is_direct_route(L3_route_t *l3_route) {
    return l3_route->is_direct ;
}


static void
layer3_pkt_receieve_from_top(node_t *node, char *pkt,
        unsigned int size, int protocol_number,
        unsigned int dest_ip_address) {

            ip_hdr_t ip_hdr; 
            init_ip_hdr(&ip_hdr);

            ip_hdr.protocol = protocol_number;
            ip_hdr.dest_ip = dest_ip_address;

            unsigned int addr_int = 0 ; 
            inet_pton(AF_INET, NODE_LOOPBACKADDRESS(node), &addr_int);
            addr_int = htonl(addr_int);
            ip_hdr.src_ip = addr_int; 

            ip_hdr.total_length = (short) ip_hdr.header_length + (short) (size/4) +  (short)((size % 4) ? 1 : 0);

            L3_route_t * l3_route = l3rib_lookup_route(Node_RT_TABLE(node), ip_hdr.dest_ip);

             if(!l3_route){
               printf("Node : %s : No L3 route\n",  node->node_name); 
                return;
               }
            
               char *new_pkt = NULL ; 
               unsigned int new_pkt_size = 0 ;

               new_pkt_size = ip_hdr.total_length*4 ; 
               new_pkt = calloc(1, MAX_PACKET_BUFFER_SIZE);

               memcpy(new_pkt, (char*)&ip_hdr, ip_hdr.header_length * 4);
               
               if (pkt && size) {
                memcpy(new_pkt + (ip_hdr.header_length * 4), pkt, size);
               }
            
               bool_t is_direct_route = l3_is_direct_route(l3_route);

               unsigned int next_hop_ip  ; 

               if(!is_direct_route) {
                     inet_pton(AF_INET, l3_route->gw_ip, &next_hop_ip);
                     next_hop_ip = htonl(next_hop_ip);
               } else {
                 /*Case 2 : Direct Host Delivery Case*/
                 /*Case 4 : Self-Ping Case*/
                /* The Data link layer will differentiate between case 2 
                  * and case 4 and take appropriate action*/
                        next_hop_ip = dest_ip_address ; 
               }

               char *shifted_pkt_buffer = pkt_buffer_shift_right(new_pkt,new_pkt_size, MAX_PACKET_BUFFER_SIZE);

               demote_pkt_to_layer2(node,next_hop_ip,is_direct_route ? 0 :l3_route->if_name,shifted_pkt_buffer,new_pkt_size,ETH_IP);

               free(new_pkt);


        }



 bool_t
is_layer3_local_delivery(node_t *node, unsigned int dst_ip) {

    char dest_ip_str[16] ;
    dest_ip_str[15] = '\0';
    char *intf_addr = NULL ; 

    dst_ip = htonl(dst_ip);
    inet_ntop(AF_INET, &dst_ip, dest_ip_str, 16);

    if (strncmp(NODE_LOOPBACKADDRESS(node),dest_ip_str,16) == 0 ) return TRUE ; 


    /* check with the interface */

    unsigned int i = 0 ; 

    interface_t *intf ; 
    for ( ; i<MAX_INTF_PER_NODE; i++) {
        intf = node->intf[i];

        if (!intf) return FALSE ; 

        if (intf->interface_nw_props.is_ip_address_config == FALSE) continue;

        intf_addr = INTERFACE_IP(intf);

        if (strncmp(intf_addr,dest_ip_str,16) == 0) return TRUE ; 
    }
    return FALSE ; 
    
}




static void layer3_ip_pkt_recv_from_bottom(node_t *node, interface_t *interface,
        ip_hdr_t *pkt, unsigned int pkt_size) {

            char *l4_hdr, *l5_hdr ; 
            char dest_ip_addr[16];

            ip_hdr_t *ip_hdr = pkt ; 

            unsigned int dest_ip = htonl(ip_hdr->dest_ip);
            inet_ntop(AF_INET, &dest_ip, dest_ip_addr,16);

            L3_route_t *l3_route = l3rib_lookup_route(Node_RT_TABLE(node),ip_hdr->dest_ip);

            if (!l3_route) {

                printf("Router %s : Cannot route IP : %s  because no entry is not present in the routing table\n",node->node_name, dest_ip_addr);

                return ; 
            }



        /*
        
        
        L3 ROUTE EXIST 

        CASE 1 : Pkt is destined to the self (for this router only)
        CASE 2 : Pkt is destined for the  host machine connected to directly attached subnet
        CASE 3 : Pkt is to be forwaded to the next router 
        
        

        
        */



/*
CASE 1 and CASE 2 are possible 
*/


if (l3_is_direct_route(l3_route)) {

    if (is_layer3_local_delivery(node,ip_hdr->dest_ip)) {

        switch(ip_hdr->protocol) {
            case ICMP_PRO:
            printf("IP Address : %s, ping success\n", dest_ip_addr);
                    break;
            default:
            ;
        }

    }


     /* case 2 : It means, the dst ip address lies in direct connected
         * subnet of this router, time for l2 routing*/

    demote_pkt_to_layer2 (
        node,0,NULL,(char *)ip_hdr,pkt_size,ETH_IP
    );
    return ; 

}


 /*case 3 : L3 forwarding case*/

 ip_hdr->ttl-- ; 


if (ip_hdr->ttl == 0 ) {
    /*
    drop the packet 
    */
   return;
}

unsigned int nextHopIP ;

inet_pton(AF_INET, l3_route->gw_ip, &nextHopIP);

nextHopIP = htonl(nextHopIP);



demote_pkt_to_layer2(

    node,nextHopIP,l3_route->if_name,(char *)ip_hdr,pkt_size, ETH_IP
);


   }


void
init_rt_table(rt_table_t **rt_table) {
    *rt_table = calloc(1, sizeof(rt_table_t));
    init_glthread(&((*rt_table)->route_list));
    
}


L3_route_t *
rt_table_lookup(rt_table_t *rt_table, char *ip_addr, char mask){
    
    glthread_t *curr;
    L3_route_t *l3_route;

    ITERATE_GLTHREAD_BEGIN(&rt_table->route_list, curr){

        l3_route = rt_glue_to_l3_route(curr);
        if(strncmp(l3_route->dest, ip_addr, 16) == 0 && 
                l3_route->mask == mask){
            return l3_route;
        }
    } ITERATE_GLTHREAD_END(&rt_table->route_list, curr);
}


L3_route_t *l3rib_lookup_route(rt_table_t *rt_table, unsigned int dest_ip) {

   L3_route_t *l3_route = NULL;
   L3_route_t  *lpm_l3_route = NULL;
   L3_route_t  *default_l3_rt = NULL;

   glthread_t *curr = NULL;

    char subnet[16];
    char dest_ip_str[16];
    char longest_mask = 0;

    dest_ip = htonl(dest_ip);
    inet_ntop(AF_INET,&dest_ip, dest_ip_str, 16);
    dest_ip_str[15] = '\0';

    ITERATE_GLTHREAD_BEGIN(&rt_table->route_list, curr) {
        l3_route = rt_glue_to_l3_route(curr);
        memset(subnet, 0, 16);
        apply_mask(dest_ip_str, l3_route->mask, &subnet);

         if(strncmp("0.0.0.0", l3_route->dest, 16) == 0 &&
                l3_route->mask == 0){
            default_l3_rt = l3_route;
        }
        else if(strncmp(subnet, l3_route->dest, strlen(subnet)) == 0){
            if( l3_route->mask > longest_mask){
                longest_mask = l3_route->mask;
                lpm_l3_route = l3_route;
            }
        }

    } ITERATE_GLTHREAD_END(&rt_table->route_list, curr);
    return lpm_l3_route ? lpm_l3_route : default_l3_rt;
                
}



void delete_rt_table_entry(rt_table_t *rt_table, 
        char *ip_addr, char mask){

    char dst_str_with_mask[16];
    
    apply_mask(ip_addr, mask, dst_str_with_mask); 
    L3_route_t *l3_route = rt_table_lookup(rt_table, dst_str_with_mask, mask);

    if(!l3_route)
        return;

    //remove_glthread(&l3_route->route_glue);
    //free(l3_route);
}






static bool_t _rt_table_entry_add(rt_table_t *rt_table, L3_route_t *l3_route) {
    L3_route_t *l3_route_old = rt_table_lookup(rt_table, l3_route->dest, l3_route->mask) ;
    
    if (l3_route_old && IS_L3_ROUTE_EQUAL(l3_route_old, l3_route)) {
        /* Route already exists */
        return FALSE;
    }

    if (l3_route_old) {
        /* Remove the old route */
       delete_rt_table_entry(rt_table,l3_route_old->dest, l3_route_old->mask);
        
    }

    init_glthread(&l3_route->route_glue);
    glthread_add_next(&rt_table->route_list, &l3_route->route_glue);
    return TRUE ; 

}




void rt_table_add_route(rt_table_t *rt_table,
                          char *dst, char mask,
                          char *gw, char *oif) {

unsigned int dst_int ; 
char dst_str_with_mask[16];

apply_mask(dst, mask, dst_str_with_mask);

inet_pton(AF_INET, dst_str_with_mask, &dst_int);

printf("Debug: After mask applied: %s/%d\n", dst_str_with_mask, mask);

L3_route_t *l3_route = l3rib_lookup_route(rt_table, dst_int); 


/*Trying to add duplicate route!!*/
//    assert(!l3_route);

    if (l3_route) {
        printf("Route %s/%d already exists, skipping add.\n", dst_str_with_mask, mask);
        return;
    }

   l3_route = calloc(1, sizeof(L3_route_t));
   strncpy(l3_route->dest, dst_str_with_mask, 16);
   l3_route->dest[15] = '\0';
   l3_route->mask = mask;

   if (!gw && !oif) {
    l3_route->is_direct = TRUE; 
   } else {
    l3_route->is_direct = FALSE;
   }

   if (gw && oif) {
    strncpy(l3_route->gw_ip,gw,16);
    l3_route->gw_ip[15] = '\0';
    strncpy(l3_route->if_name, oif, IF_NAME_SIZE);
    l3_route->if_name[IF_NAME_SIZE - 1] = '\0';
   }

    if(!_rt_table_entry_add(rt_table, l3_route)){ /* TO DO */
        printf("Error : Route %s/%d Installation Failed\n", 
            dst_str_with_mask, mask);
        free(l3_route);   
   } 

}





void rt_table_add_direct_route(rt_table_t *rt_table,
                          char *dst, char mask) {
    rt_table_add_route(rt_table, dst, mask, 0, 0);

}





void
dump_rt_table(rt_table_t *rt_table) {
printf("Dumping L3 Routing Table\n");

   glthread_t *curr = NULL ;
  L3_route_t *l3_route = NULL;    

ITERATE_GLTHREAD_BEGIN(&rt_table->route_list, curr){
    l3_route = rt_glue_to_l3_route(curr);
    printf("Debug: dest='%s' mask=%d is_direct=%d gw_ip='%s' oif='%s'\n",
        l3_route->dest, l3_route->mask, l3_route->is_direct,
        l3_route->gw_ip, l3_route->if_name);

    printf("\t%-18s %-4d %-18s %s\n", 
        l3_route->dest, l3_route->mask,
        l3_route->is_direct ? "NA" : l3_route->gw_ip, 
        l3_route->is_direct ? "NA" : l3_route->if_name);
} ITERATE_GLTHREAD_END(&rt_table->route_list, curr);


}


void layer3_pkt_recv_from_bottom(node_t *node, interface_t *interface ,char *pkt, unsigned int pkt_size, int protocol_number) {
    switch(protocol_number) {
        case ETH_IP :
          layer3_ip_pkt_recv_from_bottom(node, interface, (ip_hdr_t *)pkt, pkt_size);
            break;
        default:
            ;
    }

}




void demote_pkt_to_layer3(node_t *node, char *pkt, unsigned int size, 
    int protocol_num, // l4 or l5 protocol number
    unsigned int dest_ip) {
    layer3_pkt_receieve_from_top(node, pkt, size, protocol_num, dest_ip);

}


void promote_pkt_to_layer3(node_t *node, interface_t *interface ,char *pkt, unsigned int pkt_size, int protocol_number) {

    layer3_pkt_recv_from_bottom(node,interface,pkt,pkt_size,protocol_number);
    
}



