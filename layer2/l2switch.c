#include "glthread.h"
#include "net.h"
#include "graph.h"



typedef struct mac_table_entries {
    mac_address_t mac_address;
    char oif_name[IF_NAME_SIZE];
    glthread_t mac_glue;
} mac_table_entries_t;




typedef struct mac_table_ {
    glthread_t mac_entries;
} mac_table_t;