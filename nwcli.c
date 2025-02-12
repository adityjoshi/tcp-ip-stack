#include "CommandParser/libcli.h"
#include "CommandParser/cmdtlv.h"
#include "cmdcodes.h"
#include "graph.h"
#include <stdio.h>

extern graph_t *topo;

/* node dumping*/
void dump_nw_node(param_t *param, ser_buff_t *tlv_buf) {
    node_t *node ;
    glthread_t *curr;
    ITERATE_GLTHREAD_BEGIN(&topo->node_list,curr) {
        node = graph_glue_to_node(curr);
        printf("Node Name = %s\n",node->node_name);
    } ITERATE_GLTHREAD_END(&topo->node_list,curr);
}



/*
arp_handler
*/

static int arp_handler(param_t *param, ser_buff_t *tlv_buf,
    op_mode enable_or_disable) {
        node_t *node;
        char *node_name;
        char *ip_addr;
        tlv_struct_t *tlv = NULL;
        
    }




/*General validations and checks */
int valid_node_existence(char *node_name) {
    node_t *node = get_node_by_node_name(topo,node_name);
    if(node) 
    return VALIDATION_SUCCESS;
    printf("Error : Node %s does not exist\n",node_name);
    return VALIDATION_FAILED;
}


/*Generic Topology Commands*/
static int
show_nw_topology_handler(param_t *param, ser_buff_t *tlv_buf, op_mode enable_or_disable){

    int CMDCODE = -1;
    CMDCODE = EXTRACT_CMD_CODE(tlv_buf);

    switch(CMDCODE){

        case CMDCODE_SHOW_NW_TOPOLOGY:
            dump_nw_graph(topo);
            break;
        default:
            ;
    }
    return 0;
}


void nw_init_cli() {
    init_libcli();

    param_t *show   = libcli_get_show_hook();
    param_t *debug  = libcli_get_debug_hook();
    param_t *config = libcli_get_config_hook();
    param_t *run    = libcli_get_run_hook();
    param_t *debug_show = libcli_get_debug_show_hook();
    param_t *root = libcli_get_root();

     {
        /*show topology*/
         static param_t topology;
         init_param(&topology, CMD, "topology", show_nw_topology_handler, 0, INVALID, 0, "Dump Complete Network Topology");
         libcli_register_param(show, &topology);
         set_param_cmd_code(&topology, CMDCODE_SHOW_NW_TOPOLOGY);
     }

     {
        /* run node*/
        static param_t node;
        init_param(&node, CMD, "node", NULL, 0, INVALID, 0, "\"node\" keyword"); 
        libcli_register_param(run, &node);
        libcli_register_display_callback(&node, dump_nw_node);
        {
             /*run node name*/
             static param_t node_name;
             init_param(&node_name, LEAF, 0, 0,valid_node_existence, INVALID, 0, "Node Name");
             libcli_register_param(&node, &node_name);

             {
                static param_t resolve_arp;
                init_param(&resolve_arp, CMD, "resolve-arp", NULL, 0, INVALID, 0, "Resolve ARP for this node");
                libcli_register_param(&node_name, &resolve_arp);

                {
                    // run node <node-name> resolve-arp <ip-address>
                    static param_t ip_addr;
                    init_param(&ip_addr, LEAF, 0, arp_handler, 0, INVALID, 0, "IP Address");
                    set_param_cmd_code(&ip_addr, CMDCODE_RUN_ARP);
                }
             }

        }
     }
     
}