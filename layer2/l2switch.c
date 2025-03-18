#include "gluethread/glthread.h"
#include "net.h"
#include "graph.h"
#include <stdio.h>
#include <stdlib.h>


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


// void layer2_switch_recv_frame(interface_t *interface, char *pkt, unsigned int pkt_size) {

// }


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
