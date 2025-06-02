
#ifndef __LAYER3__
#define __LAYER3__

#include "gluethread/glthread.h"
#include "utils.h"
#define IF_NAME_SIZE 16



#pragma pack(push, 1)

typedef struct ip_hdr {
    unsigned int version : 4 ; 
    unsigned int header_length :4 ; 
    char tos ; 
    unsigned short total_length ; 
    
    unsigned short identification ;
    unsigned int unused_flag : 1 ;
    unsigned int Dont_Fragment_flag : 1;   
    unsigned int MORE_flag : 1; 
    unsigned int frag_offset : 13;

    char ttl ;
    char protocol ;
    unsigned short checksum ;
    unsigned int src_ip ;
    unsigned int dest_ip ; 

}ip_hdr_t ; 


#pragma pack(pop)

static inline void init_ip_hdr(ip_hdr_t *ip_hdr) {
    ip_hdr->version = 4 ; 
    ip_hdr->header_length = 5 ; /*no use of option field so it will remain only 5*4=20bytes*/
    ip_hdr->tos = 0 ; 
    ip_hdr->total_length = 0;

    ip_hdr->identification = 0 ;
    ip_hdr->unused_flag = 0;
    ip_hdr->Dont_Fragment_flag = 1 ; 
    ip_hdr->MORE_flag = 0 ;
    ip_hdr->frag_offset = 0 ;


    ip_hdr->ttl = 64 ;
    ip_hdr->protocol = 0 ; 
    ip_hdr->checksum = 0 ;
    ip_hdr->src_ip = 0 ; 
    ip_hdr->dest_ip = 0 ; 

}



typedef struct rt_table {
    glthread_t route_list; 
} rt_table_t ; 


typedef struct L3_route {
    char dest[16] ; 
    char mask ;
    bool_t is_direct ; /* for local route and direct route*/
    char gw_ip[16]; /* next gateway ip */
     char if_name[IF_NAME_SIZE]; /* outgoing interface name */
    glthread_t route_glue;
} L3_route_t;


GLTHREAD_TO_STRUCT(rt_glue_to_l3_route, L3_route_t, route_glue);


void
init_rt_table(rt_table_t **rt_table);


void rt_table_add_direct_route(rt_table_t *rt_table,
                          char *dst, char mask);


void rt_table_add_route(rt_table_t *rt_table,
                          char *dst, char mask,
                          char *gw, char *oif);



L3_route_t *
l3rib_lookup_route(rt_table_t *rt_table,
                 unsigned int dest_ip);


void
dump_rt_table(rt_table_t *rt_table);


void
delete_rt_table_entry(rt_table_t *rt_table, char *ip_addr, char mask);




#define IS_L3_ROUTE_EQUAL(rt1, rt2) \
    (strncmp((rt1)->dest, (rt2)->dest, 16) == 0 && \
     (rt1)->mask == (rt2)->mask && \
     (rt1)->is_direct == (rt2)->is_direct && \
     strncmp((rt1)->gw_ip, (rt2)->gw_ip, 16) == 0 && \
     strncmp((rt1)->if_name, (rt2)->if_name, IF_NAME_SIZE) == 0)










/*

MACROS 

*/




#define IP_HDR_LEN_BYTES(ip_hdr_ptr) (ip_hdr_ptr->header_length * 4)
#define IP_HDR_TOTAL_LEN_IN_BYTES(ip_hdr_ptr) (ip_hdr_ptr->total_length)
#define RETURN_PAYLOAD_START(ip_hdr_ptr) ((char *)ip_hdr_ptr + IP_HDR_LEN_BYTES(ip_hdr_ptr))
#define IP_HDR_PAYLOAD_SIZE(ip_hdr_ptr) (IP_HDR_TOTAL_LEN_IN_BYTES(ip_hdr_ptr) - IP_HDR_LEN_BYTES(ip_hdr_ptr))






#endif /* __LAYER3__ */