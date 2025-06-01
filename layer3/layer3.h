#ifndef __LAYER3__
#define __LAYER3__


#include <glthread.h>
#include "utils.h"
#include "graph.h"

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






#define IS_L3_ROUTE_EQUAL(rt1, rt2) \
    (strncmp((rt1)->dest, (rt2)->dest, 16) == 0 && \
     (rt1)->mask == (rt2)->mask && \
     (rt1)->is_direct == (rt2)->is_direct && \
     strncmp((rt1)->gw_ip, (rt2)->gw_ip, 16) == 0 && \
     strncmp((rt1)->if_name, (rt2)->if_name, IF_NAME_SIZE) == 0)
























#endif /* __LAYER3__ */