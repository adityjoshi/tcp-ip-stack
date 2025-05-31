#include "layer3/layer3.h"




void
init_rt_table(rt_table_t **rt_table) {
    *rt_table = calloc(1, sizeof(rt_table_t));
    init_glthread(&((*rt_table)->route_list));
    
}



void rt_table_add_direct_route(rt_table_t *rt_table,
                          char *dst, char mask) {
    rt_table_add_route(rt_table, dst, mask, 0, 0);

}