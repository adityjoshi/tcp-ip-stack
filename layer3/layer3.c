#include "layer3/layer3.h"
#include "arpa/inet.h"



void
init_rt_table(rt_table_t **rt_table) {
    *rt_table = calloc(1, sizeof(rt_table_t));
    init_glthread(&((*rt_table)->route_list));
    
}


void rt_table_add_route(rt_table_t *rt_table,
                          char *dst, char mask,
                          char *gw, char *oif) {

unsigned int dst_int = 0 ; 
char dst_str_with_mask[16];

apply_mask(dst, mask, &dst_str_with_mask);

inet_pton(AF_INET, dst_str_with_mask, &dst_int);


L3_route_t *l3_route = l3rib_lookup_route(rt_table, dst_int);


/*Trying to add duplicate route!!*/
   assert(!l3_route);

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

    if(!_rt_table_entry_add(rt_table, l3_route)){
        printf("Error : Route %s/%d Installation Failed\n", 
            dst_str_with_mask, mask);
        free(l3_route);   
   } 

}





void rt_table_add_direct_route(rt_table_t *rt_table,
                          char *dst, char mask) {
    rt_table_add_route(rt_table, dst, mask, 0, 0);

}



