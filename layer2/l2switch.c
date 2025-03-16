#include "glthread.h"
#include "net.h"
#include "graph.h"



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
}

