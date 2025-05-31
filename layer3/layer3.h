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
    char oif_name[IF_NAME_SIZE]; /* outgoing interface name */
    glthread_t route_glue;
} L3_route_t;













































#endif /* __LAYER3__ */