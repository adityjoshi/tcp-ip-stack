#include "layer2.h"








void
init_arp_table(arp_table_t **arp_table){

    *arp_table = calloc(1, sizeof(arp_table_t));
    init_glthread(&((*arp_table)->arp_entries));
}


void arp_table_entry_lookup(arp_table_t *arp_table, char *ip_addr) {
    
}
