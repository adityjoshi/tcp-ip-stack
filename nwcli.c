#include "CommandParser/libcli.h"
#include "CommandParser/cmdtlv.h"
#include "cmdcodes.h"
#include "graph.h"
#include <stdio.h>
#include "layer3/layer3.h"
#include "layer2/layer2.h"
#include <unistd.h>
#include <arpa/inet.h>

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
typedef struct arp_table_ arp_table_t;
extern void
dump_arp_table(arp_table_t *arp_table);

static int show_arp_handler(param_t *param, ser_buff_t *tlv_buf,
    op_mode enable_or_disable) {

        node_t *node;
        char *node_name;
       // char *ip_addr;
        tlv_struct_t *tlv = NULL;


        TLV_LOOP_BEGIN(tlv_buf, tlv) {
            if(strncmp(tlv->leaf_id, "node-name", strlen("node-name")) ==0)
            node_name = tlv->value;
        } TLV_LOOP_END;
        node = get_node_by_node_name(topo, node_name);
        if (!node) {
            printf("Error: Node %s does not exist\n", node_name);
            return -1;
        }
//         arp_table_t *arp_table = NODE_ARP_TABLE(node);
// if (!arp_table) {
//     printf("Error: ARP table for node %s is NULL\n", node->node_name);
//     return -1;
// }
dump_arp_table(NODE_ARP_TABLE(node));
        return 0;
        
    }


typedef struct mac_table_ mac_table_t;  
extern void
dump_mac_table(mac_table_t *mac_table);
static int
show_mac_handler(param_t *param, ser_buff_t *tlv_buf,
                    op_mode enable_or_disable){

    node_t *node;
    char *node_name;
    tlv_struct_t *tlv = NULL;
    
    TLV_LOOP_BEGIN(tlv_buf, tlv){

        if(strncmp(tlv->leaf_id, "node-name", strlen("node-name")) ==0)
            node_name = tlv->value;

    }TLV_LOOP_END;

    node = get_node_by_node_name(topo, node_name);
    dump_mac_table(NODE_MAC_TABLE(node));
    return 0;

}

static int show_stp_handler(param_t *param, ser_buff_t *tlv_buf,
                    op_mode enable_or_disable) {
                       
    node_t *node;
    char *node_name;
    tlv_struct_t *tlv = NULL;
    
    TLV_LOOP_BEGIN(tlv_buf, tlv){

        if(strncmp(tlv->leaf_id, "node-name", strlen("node-name")) ==0)
            node_name = tlv->value;

    }TLV_LOOP_END;

    node = get_node_by_node_name(topo, node_name);
    extern void stp_dump(node_t *node);
    stp_dump(node);
    return 0;
                    
}

static int find_paths_handler(param_t *param, ser_buff_t *tlv_buf,
                    op_mode enable_or_disable) {
                       
    node_t *dst_node;
    char *node_name = NULL;
    tlv_struct_t *tlv = NULL;
    
    TLV_LOOP_BEGIN(tlv_buf, tlv){
        if(strncmp(tlv->leaf_id, "node-name", strlen("node-name")) ==0) {
            node_name = tlv->value;
            break;
        }
    }TLV_LOOP_END;

    if (!node_name) {
        printf("Error: Node name not provided\n");
        return 0;
    }

    dst_node = get_node_by_node_name(topo, node_name);
    if (!dst_node) {
        printf("Error: Node %s not found\n", node_name);
        return 0;
    }
    
    extern void find_and_configure_paths_to_node(node_t *dst_node);
    find_and_configure_paths_to_node(dst_node);
    return 0;
                    
}

// Manual page handler
static int man_handler(param_t *param, ser_buff_t *tlv_buf,
                       op_mode enable_or_disable) {
    tlv_struct_t *tlv = NULL;
    char *cmd_name = NULL;
    
    TLV_LOOP_BEGIN(tlv_buf, tlv) {
        if (strncmp(tlv->leaf_id, "command-name", strlen("command-name")) == 0) {
            cmd_name = tlv->value;
        }
    } TLV_LOOP_END;
    
    if (!cmd_name) {
        // Show list of all commands
        printf("\n=== TCP/IP Stack Command Manual ===\n\n");
        printf("Available commands:\n");
        printf("  show topology\n");
        printf("  show node <node-name> arp\n");
        printf("  show node <node-name> mac\n");
        printf("  show node <node-name> rt\n");
        printf("  show node <node-name> stp\n");
        printf("  run node <node-name> ping <ip-address>\n");
        printf("  run node <node-name> resolve-arp <ip-address>\n");
        printf("  run arp-resolve-all\n");
        printf("  run route-configure-all\n");
        printf("  run find-paths <node-name>\n");
        printf("  config node <node-name> interface <if-name> l2mode <access|trunk>\n");
        printf("  config node <node-name> interface <if-name> vlan <vlan-id>\n");
        printf("  config node <node-name> route <ip-address> <mask> [<gw-ip> <oif>]\n");
        printf("\nUse 'man <command>' for detailed help on a specific command.\n");
        printf("Example: man ping\n");
        return 0;
    }
    
    // Show manual for specific command
    if (strcmp(cmd_name, "topology") == 0 || strcmp(cmd_name, "show-topology") == 0) {
        printf("\n=== MANUAL: show topology ===\n\n");
        printf("NAME\n");
        printf("    show topology - Display the complete network topology\n\n");
        printf("SYNOPSIS\n");
        printf("    show topology\n\n");
        printf("DESCRIPTION\n");
        printf("    Displays a visual representation of the entire network topology,\n");
        printf("    showing all nodes, their interfaces, IP addresses, MAC addresses,\n");
        printf("    and connections between nodes.\n\n");
        printf("EXAMPLES\n");
        printf("    tcp-ip-project> show topology\n\n");
        return 0;
    }
    
    if (strcmp(cmd_name, "arp") == 0 || strcmp(cmd_name, "show-arp") == 0) {
        printf("\n=== MANUAL: show node <node-name> arp ===\n\n");
        printf("NAME\n");
        printf("    show node <node-name> arp - Display ARP table for a node\n\n");
        printf("SYNOPSIS\n");
        printf("    show node <node-name> arp\n\n");
        printf("DESCRIPTION\n");
        printf("    Displays the Address Resolution Protocol (ARP) table for the\n");
        printf("    specified node. The ARP table maps IP addresses to MAC addresses\n");
        printf("    for devices on the local network.\n\n");
        printf("PARAMETERS\n");
        printf("    <node-name>  Name of the node whose ARP table to display\n\n");
        printf("EXAMPLES\n");
        printf("    tcp-ip-project> show node R1 arp\n");
        printf("    tcp-ip-project> show node H1 arp\n\n");
        return 0;
    }
    
    if (strcmp(cmd_name, "mac") == 0 || strcmp(cmd_name, "show-mac") == 0) {
        printf("\n=== MANUAL: show node <node-name> mac ===\n\n");
        printf("NAME\n");
        printf("    show node <node-name> mac - Display MAC address table for a switch\n\n");
        printf("SYNOPSIS\n");
        printf("    show node <node-name> mac\n\n");
        printf("DESCRIPTION\n");
        printf("    Displays the MAC address forwarding table for a Layer 2 switch.\n");
        printf("    This table maps MAC addresses to interfaces, allowing the switch\n");
        printf("    to forward frames to the correct port.\n\n");
        printf("PARAMETERS\n");
        printf("    <node-name>  Name of the switch node\n\n");
        printf("EXAMPLES\n");
        printf("    tcp-ip-project> show node SW1 mac\n");
        printf("    tcp-ip-project> show node SW2 mac\n\n");
        return 0;
    }
    
    if (strcmp(cmd_name, "rt") == 0 || strcmp(cmd_name, "route") == 0 || strcmp(cmd_name, "routing") == 0) {
        printf("\n=== MANUAL: show node <node-name> rt ===\n\n");
        printf("NAME\n");
        printf("    show node <node-name> rt - Display routing table for a node\n\n");
        printf("SYNOPSIS\n");
        printf("    show node <node-name> rt\n\n");
        printf("DESCRIPTION\n");
        printf("    Displays the Layer 3 routing table for the specified node.\n");
        printf("    The routing table contains routes to different network destinations,\n");
        printf("    including next-hop gateways and output interfaces.\n\n");
        printf("PARAMETERS\n");
        printf("    <node-name>  Name of the router node\n\n");
        printf("EXAMPLES\n");
        printf("    tcp-ip-project> show node R1 rt\n");
        printf("    tcp-ip-project> show node R2 rt\n\n");
        return 0;
    }
    
    if (strcmp(cmd_name, "stp") == 0 || strcmp(cmd_name, "show-stp") == 0) {
        printf("\n=== MANUAL: show node <node-name> stp ===\n\n");
        printf("NAME\n");
        printf("    show node <node-name> stp - Display Spanning Tree Protocol status\n\n");
        printf("SYNOPSIS\n");
        printf("    show node <node-name> stp\n\n");
        printf("DESCRIPTION\n");
        printf("    Displays the Spanning Tree Protocol (STP) status for a switch.\n");
        printf("    Shows bridge ID, root bridge information, and port states\n");
        printf("    (Forwarding, Blocking, etc.) and roles (Root, Designated, Alternate).\n\n");
        printf("PARAMETERS\n");
        printf("    <node-name>  Name of the switch node\n\n");
        printf("EXAMPLES\n");
        printf("    tcp-ip-project> show node SW1 stp\n");
        printf("    tcp-ip-project> show node SW2 stp\n\n");
        return 0;
    }
    
    if (strcmp(cmd_name, "ping") == 0) {
        printf("\n=== MANUAL: run node <node-name> ping <ip-address> ===\n\n");
        printf("NAME\n");
        printf("    run node <node-name> ping <ip-address> - Send ICMP echo request\n\n");
        printf("SYNOPSIS\n");
        printf("    run node <node-name> ping <ip-address>\n\n");
        printf("DESCRIPTION\n");
        printf("    Sends an ICMP echo request (ping) from the specified node to the\n");
        printf("    destination IP address. This is used to test network connectivity\n");
        printf("    and verify that a host is reachable.\n\n");
        printf("PARAMETERS\n");
        printf("    <node-name>   Source node that will send the ping\n");
        printf("    <ip-address>  Destination IPv4 address (e.g., 10.1.1.2)\n\n");
        printf("EXAMPLES\n");
        printf("    tcp-ip-project> run node H1 ping 10.1.1.2\n");
        printf("    tcp-ip-project> run node R1 ping 20.1.1.1\n\n");
        printf("NOTES\n");
        printf("    - Ensure ARP tables are built before pinging (use 'run arp-resolve-all')\n");
        printf("    - Ping uses ICMP protocol at Layer 3\n");
        printf("    - Routing tables must be configured for inter-subnet pings\n\n");
        return 0;
    }
    
    if (strcmp(cmd_name, "resolve-arp") == 0 || strcmp(cmd_name, "arp-resolve") == 0) {
        printf("\n=== MANUAL: run node <node-name> resolve-arp <ip-address> ===\n\n");
        printf("NAME\n");
        printf("    run node <node-name> resolve-arp <ip-address> - Resolve ARP for an IP address\n\n");
        printf("SYNOPSIS\n");
        printf("    run node <node-name> resolve-arp <ip-address>\n\n");
        printf("DESCRIPTION\n");
        printf("    Manually triggers ARP resolution for a specific IP address on a node.\n");
        printf("    Sends an ARP request to discover the MAC address associated with the\n");
        printf("    given IP address.\n\n");
        printf("PARAMETERS\n");
        printf("    <node-name>   Node that will perform ARP resolution\n");
        printf("    <ip-address>  IP address to resolve (e.g., 10.1.1.2)\n\n");
        printf("EXAMPLES\n");
        printf("    tcp-ip-project> run node H1 resolve-arp 10.1.1.2\n");
        printf("    tcp-ip-project> run node R1 resolve-arp 20.1.1.1\n\n");
        return 0;
    }
    
    if (strcmp(cmd_name, "arp-resolve-all") == 0) {
        printf("\n=== MANUAL: run arp-resolve-all ===\n\n");
        printf("NAME\n");
        printf("    run arp-resolve-all - Build ARP tables for all nodes\n\n");
        printf("SYNOPSIS\n");
        printf("    run arp-resolve-all\n\n");
        printf("DESCRIPTION\n");
        printf("    Automatically builds ARP tables for all nodes in the topology by\n");
        printf("    resolving ARP for:\n");
        printf("    - All directly connected neighbors\n");
        printf("    - All gateway IPs in routing tables\n\n");
        printf("    This is a convenient way to populate ARP tables before testing\n");
        printf("    network connectivity with ping.\n\n");
        printf("EXAMPLES\n");
        printf("    tcp-ip-project> run arp-resolve-all\n\n");
        printf("NOTES\n");
        printf("    - Should be run after topology is configured\n");
        printf("    - Required before pinging between nodes\n\n");
        return 0;
    }
    
    if (strcmp(cmd_name, "route-configure-all") == 0 || strcmp(cmd_name, "route-config-all") == 0) {
        printf("\n=== MANUAL: run route-configure-all ===\n\n");
        printf("NAME\n");
        printf("    run route-configure-all - Configure routing tables for all nodes\n\n");
        printf("SYNOPSIS\n");
        printf("    run route-configure-all\n\n");
        printf("DESCRIPTION\n");
        printf("    Automatically configures routing tables for all nodes with L3 interfaces\n");
        printf("    by adding direct routes to all directly connected neighbor subnets.\n");
        printf("    For each L3 interface, it:\n");
        printf("    - Finds the directly connected neighbor\n");
        printf("    - Calculates the neighbor's subnet from its IP and mask\n");
        printf("    - Adds a direct route to that subnet\n\n");
        printf("    This ensures all nodes can reach their directly connected neighbors\n");
        printf("    without manual route configuration.\n\n");
        printf("EXAMPLES\n");
        printf("    tcp-ip-project> run route-configure-all\n\n");
        printf("NOTES\n");
        printf("    - Only configures direct routes (no gateways)\n");
        printf("    - Skips routes that already exist\n");
        printf("    - Only processes nodes with L3 interfaces\n");
        printf("    - Should be run after topology is built and IPs are configured\n");
        printf("    - Use 'run find-paths <node>' for inter-subnet routing\n\n");
        return 0;
    }
    
    if (strcmp(cmd_name, "find-paths") == 0 || strcmp(cmd_name, "pathfinder") == 0) {
        printf("\n=== MANUAL: run find-paths <node-name> ===\n\n");
        printf("NAME\n");
        printf("    run find-paths <node-name> - Find shortest paths and prevent loops\n\n");
        printf("SYNOPSIS\n");
        printf("    run find-paths <node-name>\n\n");
        printf("DESCRIPTION\n");
        printf("    Uses Breadth-First Search (BFS) to find the shortest path from all\n");
        printf("    nodes to the specified destination node. Additionally:\n");
        printf("    - Configures static routes on L3 nodes along the best paths\n");
        printf("    - Blocks redundant inter-switch ports using STP to prevent loops\n");
        printf("    - Initializes STP on switches if not already initialized\n\n");
        printf("PARAMETERS\n");
        printf("    <node-name>  Destination node name (e.g., H2, R2)\n\n");
        printf("EXAMPLES\n");
        printf("    tcp-ip-project> run find-paths H2\n");
        printf("    tcp-ip-project> run find-paths R3\n\n");
        printf("OUTPUT\n");
        printf("    Shows paths from all nodes to destination, then blocks redundant\n");
        printf("    ports to break Layer 2 loops.\n\n");
        printf("NOTES\n");
        printf("    - Essential for preventing infinite loops in switched networks\n");
        printf("    - Should be run after topology is built\n");
        printf("    - Automatically handles STP initialization\n\n");
        return 0;
    }
    
    if (strcmp(cmd_name, "l2mode") == 0 || strcmp(cmd_name, "l2-mode") == 0) {
        printf("\n=== MANUAL: config node <node-name> interface <if-name> l2mode <access|trunk> ===\n\n");
        printf("NAME\n");
        printf("    config node <node-name> interface <if-name> l2mode - Configure L2 mode\n\n");
        printf("SYNOPSIS\n");
        printf("    config node <node-name> interface <if-name> l2mode <access|trunk>\n\n");
        printf("DESCRIPTION\n");
        printf("    Configures the Layer 2 mode for an interface:\n");
        printf("    - ACCESS: For host connections, accepts untagged frames\n");
        printf("    - TRUNK: For inter-switch links, carries VLAN-tagged frames\n\n");
        printf("PARAMETERS\n");
        printf("    <node-name>   Name of the node\n");
        printf("    <if-name>    Interface name (e.g., eth0/1)\n");
        printf("    <access|trunk>  L2 mode to set\n\n");
        printf("EXAMPLES\n");
        printf("    tcp-ip-project> config node SW1 interface eth0/1 l2mode access\n");
        printf("    tcp-ip-project> config node SW1 interface eth0/2 l2mode trunk\n\n");
        return 0;
    }
    
    if (strcmp(cmd_name, "vlan") == 0) {
        printf("\n=== MANUAL: config node <node-name> interface <if-name> vlan <vlan-id> ===\n\n");
        printf("NAME\n");
        printf("    config node <node-name> interface <if-name> vlan - Configure VLAN\n\n");
        printf("SYNOPSIS\n");
        printf("    config node <node-name> interface <if-name> vlan <vlan-id>\n\n");
        printf("DESCRIPTION\n");
        printf("    Configures VLAN membership for an interface:\n");
        printf("    - For ACCESS ports: Sets the VLAN ID for untagged frames\n");
        printf("    - For TRUNK ports: Adds VLAN to the allowed VLAN list\n\n");
        printf("PARAMETERS\n");
        printf("    <node-name>   Name of the node\n");
        printf("    <if-name>    Interface name (e.g., eth0/1)\n");
        printf("    <vlan-id>    VLAN ID (1-4094)\n\n");
        printf("EXAMPLES\n");
        printf("    tcp-ip-project> config node SW1 interface eth0/1 vlan 10\n");
        printf("    tcp-ip-project> config node SW1 interface eth0/2 vlan 10\n\n");
        return 0;
    }
    
    if (strcmp(cmd_name, "route") == 0 || strcmp(cmd_name, "config-route") == 0) {
        printf("\n=== MANUAL: config node <node-name> route ===\n\n");
        printf("NAME\n");
        printf("    config node <node-name> route - Configure static route\n\n");
        printf("SYNOPSIS\n");
        printf("    config node <node-name> route <ip-address> <mask> [<gw-ip> <oif>]\n\n");
        printf("DESCRIPTION\n");
        printf("    Adds a static route to the routing table of the specified node.\n");
        printf("    Routes can be:\n");
        printf("    - Direct routes: No gateway (for directly connected networks)\n");
        printf("    - Indirect routes: With gateway IP and output interface\n\n");
        printf("PARAMETERS\n");
        printf("    <node-name>   Name of the router node\n");
        printf("    <ip-address>  Destination network IP (e.g., 10.1.1.0)\n");
        printf("    <mask>        Subnet mask in CIDR notation (e.g., 24)\n");
        printf("    <gw-ip>       (Optional) Gateway IP address for indirect routes\n");
        printf("    <oif>         (Optional) Output interface name (e.g., eth0/1)\n\n");
        printf("EXAMPLES\n");
        printf("    # Direct route\n");
        printf("    tcp-ip-project> config node R1 route 10.1.1.0 24\n\n");
        printf("    # Indirect route with gateway\n");
        printf("    tcp-ip-project> config node R1 route 20.1.1.0 24 10.1.1.2 eth0/1\n\n");
        return 0;
    }
    
    // Command not found
    printf("\nNo manual entry for '%s'\n", cmd_name);
    printf("Use 'man' without arguments to see all available commands.\n\n");
    return 0;
}



/*General validations and checks */

int
validate_node_extistence(char *node_name){

    node_t *node = get_node_by_node_name(topo, node_name);
    if(node)
        return VALIDATION_SUCCESS;
    printf("Error : Node %s do not exist\n", node_name);
    return VALIDATION_FAILED;
}


int
validate_vlan_id(char *vlan_value){

    unsigned int vlan = atoi(vlan_value);
    if(!vlan){
        printf("Error : Invalid Vlan Value\n");
        return VALIDATION_FAILED;
    }
    if(vlan >= 1 && vlan <= 4095)
        return VALIDATION_SUCCESS;

    return VALIDATION_FAILED;
}

int
validate_l2_mode_value(char *l2_mode_value){

    if((strncmp(l2_mode_value, "access", strlen("access")) == 0) || 
        (strncmp(l2_mode_value, "trunk", strlen("trunk")) == 0))
        return VALIDATION_SUCCESS;
    return VALIDATION_FAILED;
}

int
validate_mask_value(char *mask_str){

    unsigned int mask = atoi(mask_str);
    if(!mask){
        printf("Error : Invalid Mask Value\n");
        return VALIDATION_FAILED;
    }
    if(mask >= 0 && mask <= 32)
        return VALIDATION_SUCCESS;
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
        case CMDCODE_FIND_PATHS:
            find_paths_handler(param, tlv_buf, enable_or_disable);
            break;
        default:
            ;
    }
    return 0;
}


extern void
send_arp_broadcast_request(node_t *node,
                           interface_t *oif,
                           char *ip_addr);
static int
arp_handler(param_t *param, ser_buff_t *tlv_buf,
                op_mode enable_or_disable){

    node_t *node;
    char *node_name;
    char *ip_addr;
    tlv_struct_t *tlv = NULL;

    TLV_LOOP_BEGIN(tlv_buf, tlv){

        if(strncmp(tlv->leaf_id, "node-name", strlen("node-name")) ==0)
            node_name = tlv->value;
        else if(strncmp(tlv->leaf_id, "ip-address", strlen("ip-address")) ==0)
            ip_addr = tlv->value;
    } TLV_LOOP_END;

    node = get_node_by_node_name(topo, node_name);
   send_arp_broadcast_request(node, NULL, ip_addr);
  
    return 0;
}

// Handler for resolving ARP for all nodes
static int
arp_resolve_all_handler(param_t *param, ser_buff_t *tlv_buf,
                        op_mode enable_or_disable) {
    
    node_t *node;
    glthread_t *curr;
    interface_t *intf;
    L3_route_t *l3_route;
    glthread_t *route_curr;
    char ip_set[256][16];  // Store unique IPs to resolve
    int ip_count = 0;
    int i, j;
    bool_t ip_exists;
    
    printf("\n=== Building ARP tables for all nodes ===\n\n");
    
    // First pass: Collect all IPs that need ARP resolution
    ITERATE_GLTHREAD_BEGIN(&topo->node_list, curr) {
        node = graph_glue_to_node(curr);
        
        // Get IPs from interfaces (neighbor IPs)
        for (i = 0; i < MAX_INTF_PER_NODE; i++) {
            if (!node->intf[i]) break;
            intf = node->intf[i];
            
            if (IS_INTF_L3_MODE(intf) && intf->link) {
                node_t *nbr = get_nbr_node(intf);
                if (nbr) {
                    // Find neighbor's interface IP on the same link
                    interface_t *nbr_intf = NULL;
                    for (j = 0; j < MAX_INTF_PER_NODE; j++) {
                        if (!nbr->intf[j]) break;
                        if (nbr->intf[j]->link == intf->link) {
                            nbr_intf = nbr->intf[j];
                            break;
                        }
                    }
                    if (nbr_intf && IS_INTF_L3_MODE(nbr_intf)) {
                        // Add neighbor IP to set
                        ip_exists = FALSE;
                        for (j = 0; j < ip_count; j++) {
                            if (strncmp(ip_set[j], (char *)INTERFACE_IP(nbr_intf), 16) == 0) {
                                ip_exists = TRUE;
                                break;
                            }
                        }
                        if (!ip_exists && ip_count < 256) {
                            strncpy(ip_set[ip_count], (char *)INTERFACE_IP(nbr_intf), 16);
                            ip_set[ip_count][15] = '\0';
                            ip_count++;
                        }
                    }
                }
            }
        }
        
        // Get gateway IPs from routing table
        ITERATE_GLTHREAD_BEGIN(&(Node_RT_TABLE(node)->route_list), route_curr) {
            l3_route = rt_glue_to_l3_route(route_curr);
            if (!l3_route->is_direct && strncmp(l3_route->gw_ip, "0", 16) != 0 && 
                strncmp(l3_route->gw_ip, "NA", 16) != 0) {
                // Add gateway IP to set
                ip_exists = FALSE;
                for (j = 0; j < ip_count; j++) {
                    if (strncmp(ip_set[j], l3_route->gw_ip, 16) == 0) {
                        ip_exists = TRUE;
                        break;
                    }
                }
                if (!ip_exists && ip_count < 256) {
                    strncpy(ip_set[ip_count], l3_route->gw_ip, 16);
                    ip_set[ip_count][15] = '\0';
                    ip_count++;
                }
            }
        } ITERATE_GLTHREAD_END(&(Node_RT_TABLE(node)->route_list), route_curr);
    } ITERATE_GLTHREAD_END(&topo->node_list, curr);
    
    // Second pass: For each node, resolve ARP for all collected IPs
    ITERATE_GLTHREAD_BEGIN(&topo->node_list, curr) {
        node = graph_glue_to_node(curr);
        printf("Processing node: %s\n", node->node_name);
        
        for (i = 0; i < ip_count; i++) {
            // Check if this IP is in a subnet reachable from this node
            interface_t *oif = node_get_matching_subnet_interface(node, ip_set[i]);
            // Only resolve ARP if interface exists, has a link, is in L3 mode, and IP is not local
            if (oif && oif->link && IS_INTF_L3_MODE(oif) && 
                strncmp((char *)INTERFACE_IP(oif), ip_set[i], 16) != 0) {
                // Check if ARP entry already exists
                arp_entries_t *arp_entry = arp_table_entry_lookup(NODE_ARP_TABLE(node), ip_set[i]);
                if (!arp_entry) {
                    printf("  Resolving ARP for %s...\n", ip_set[i]);
                    send_arp_broadcast_request(node, oif, ip_set[i]);
                    // Small delay to allow ARP reply
                    usleep(100000);  // 100ms delay
                } else {
                    printf("  ARP entry for %s already exists\n", ip_set[i]);
                }
            }
        }
        printf("\n");
    } ITERATE_GLTHREAD_END(&topo->node_list, curr);
    
    printf("=== ARP table building complete ===\n\n");
    return 0;
}

static int route_configure_all_handler(param_t *param, ser_buff_t *tlv_buf,
                                       op_mode enable_or_disable) {
    extern graph_t *topo;
    glthread_t *curr;
    node_t *node;
    
    printf("\n=== Configuring routing tables for all nodes ===\n\n");
    
    ITERATE_GLTHREAD_BEGIN(&topo->node_list, curr) {
        node = graph_glue_to_node(curr);
        
        // Check if node has any L3 interfaces
        bool_t has_l3 = FALSE;
        unsigned int i;
        for (i = 0; i < MAX_INTF_PER_NODE; i++) {
            if (node->intf[i] && IS_INTF_L3_MODE(node->intf[i])) {
                has_l3 = TRUE;
                break;
            }
        }
        
        if (!has_l3) {
            printf("Skipping node %s (no L3 interfaces)\n", node->node_name);
            continue;
        }
        
        printf("Processing node: %s\n", node->node_name);
        
        // For each L3 interface, add routes to directly connected neighbor subnets
        for (i = 0; i < MAX_INTF_PER_NODE; i++) {
            interface_t *intf = node->intf[i];
            if (!intf || !intf->link) continue;
            
            // Only process L3 interfaces
            if (!IS_INTF_L3_MODE(intf)) continue;
            
            // Check if interface has IP configured
            if (!intf->interface_nw_props.is_ip_address_config) continue;
            
            // Get neighbor node
            node_t *nbr = get_nbr_node(intf);
            if (!nbr) continue;
            
            // Find neighbor's interface on the same link
            interface_t *nbr_intf = NULL;
            unsigned int j;
            for (j = 0; j < MAX_INTF_PER_NODE; j++) {
                if (nbr->intf[j] && nbr->intf[j]->link == intf->link) {
                    nbr_intf = nbr->intf[j];
                    break;
                }
            }
            
            if (!nbr_intf || !IS_INTF_L3_MODE(nbr_intf)) continue;
            
            // Check if neighbor interface has IP configured
            if (!nbr_intf->interface_nw_props.is_ip_address_config) continue;
            
            char *nbr_ip = (char *)INTERFACE_IP(nbr_intf);
            char nbr_mask = nbr_intf->interface_nw_props.mask;
            
            if (!nbr_ip || strncmp(nbr_ip, "0.0.0.0", 16) == 0) continue;
            
            // Calculate network address from neighbor's IP and mask
            char network[16];
            strncpy(network, nbr_ip, 16);
            network[15] = '\0';
            
            // Apply mask to get network address
            extern void apply_mask(char *ip_addr, char mask, char *output);
            char network_with_mask[16];
            apply_mask(network, nbr_mask, network_with_mask);
            
            // Check if route already exists
            extern L3_route_t *l3rib_lookup_route(rt_table_t *rt_table, unsigned int ip);
            unsigned int network_int;
            inet_pton(AF_INET, network_with_mask, &network_int);
            network_int = htonl(network_int);
            
            L3_route_t *existing_route = l3rib_lookup_route(Node_RT_TABLE(node), network_int);
            if (existing_route) {
                printf("  Route to %s/%d already exists, skipping\n", network_with_mask, nbr_mask);
                continue;
            }
            
            // Add direct route to neighbor's subnet
            extern void rt_table_add_direct_route(rt_table_t *rt_table, char *ip_addr, char mask);
            rt_table_add_direct_route(Node_RT_TABLE(node), network_with_mask, nbr_mask);
            printf("  Added route: %s/%d (direct, via %s)\n", network_with_mask, nbr_mask, intf->if_name);
        }
    } ITERATE_GLTHREAD_END(&topo->node_list, curr);
    
    printf("\n=== Routing table configuration complete ===\n\n");
    return 0;
}

static int show_rt_handler(param_t *param, ser_buff_t *tlv_buf,
                    op_mode enable_or_disable) {
                       
    node_t *node;
    char *node_name;
    tlv_struct_t *tlv = NULL;
    
    TLV_LOOP_BEGIN(tlv_buf, tlv){

        if(strncmp(tlv->leaf_id, "node-name", strlen("node-name")) ==0)
            node_name = tlv->value;

    }TLV_LOOP_END;

    node = get_node_by_node_name(topo, node_name);
    dump_rt_table(Node_RT_TABLE(node));
    return 0;
                    
}

extern void
delete_rt_table_entry(rt_table_t *rt_table,
        char *ip_addr, char mask);
extern void
rt_table_add_route(rt_table_t *rt_table,
        char *dst, char mask,
        char *gw, char *oif);

/*

L3 CONFIG HANDLER 

*/


static int
l3_config_handler(param_t *param, ser_buff_t *tlv_buf, op_mode enable_or_disable){

    node_t *node = NULL;
    char *node_name = NULL;
    char *intf_name = NULL;
    char *gwip = NULL;
    char *mask_str = NULL;
    char *dest = NULL;
    int CMDCODE = -1;

    CMDCODE = EXTRACT_CMD_CODE(tlv_buf); 
    
    tlv_struct_t *tlv = NULL;
    
    TLV_LOOP_BEGIN(tlv_buf, tlv){

        if     (strncmp(tlv->leaf_id, "node-name", strlen("node-name")) ==0)
            node_name = tlv->value;
        else if(strncmp(tlv->leaf_id, "ip-address", strlen("ip-address")) ==0)
            dest = tlv->value;
        else if(strncmp(tlv->leaf_id, "gw-ip", strlen("gw-ip")) ==0)
            gwip = tlv->value;
        else if(strncmp(tlv->leaf_id, "mask", strlen("mask")) ==0)
            mask_str = tlv->value;
        else if(strncmp(tlv->leaf_id, "oif", strlen("oif")) ==0)
            intf_name = tlv->value;
        else
            assert(0);

    }TLV_LOOP_END;

    node = get_node_by_node_name(topo, node_name);

    char mask;
    if(mask_str){
        mask = atoi(mask_str);
    }

    switch(CMDCODE){
        case CMDCODE_CONF_NODE_L3ROUTE:
            switch(enable_or_disable){
                case CONFIG_ENABLE:
                {
                    interface_t *intf;
                    if(intf_name){
                        intf = get_node_if_by_name(node, intf_name);
                        if(!intf){
                            printf("Config Error : Non-Existing Interface : %s\n", intf_name);
                            return -1;
                        }
                        if(!IS_INTF_L3_MODE(intf)){
                            printf("Config Error : Not L3 Mode Interface : %s\n", intf_name);
                            return -1;
                        }
                    }
                    rt_table_add_route(Node_RT_TABLE(node), dest, mask, gwip, intf_name);
                }
                break;
                case CONFIG_DISABLE:
                    delete_rt_table_entry(Node_RT_TABLE(node), dest, mask);
                    break;
                default:
                    ;
            }
            break;
        default:
            break;
    }
    return 0;
}



extern void layer5_ping(node_t *node,char *dst_ip_addr);

static int ping_handler(param_t *param, ser_buff_t *tlv_buf, op_mode enable_or_disable) {
     int CMDCODE;
    node_t *node;
    char *ip_addr = NULL, 
         *ero_ip_addr = NULL;
    char *node_name;

    CMDCODE = EXTRACT_CMD_CODE(tlv_buf);

    tlv_struct_t *tlv = NULL;

    TLV_LOOP_BEGIN(tlv_buf, tlv){

        if(strncmp(tlv->leaf_id, "node-name", strlen("node-name")) ==0)
            node_name = tlv->value;
        else if(strncmp(tlv->leaf_id, "ip-address", strlen("ip-address")) ==0)
            ip_addr = tlv->value;
        else
            assert(0);
    }TLV_LOOP_END;

    node = get_node_by_node_name(topo, node_name);

    switch(CMDCODE){

        case CMDCODE_PING:
            layer5_ping(node, ip_addr);
            break;
       
        default:
            ;
    }
    return 0;
}



extern void
interface_set_l2_mode(node_t *node,
                       interface_t *interface,
                       char *l2_mode_option);

extern void
interface_unset_l2_mode(node_t *node,
                         interface_t *interface,
                         char *l2_mode_option);
extern void
interface_set_vlan(node_t *node,
                    interface_t *interface,
                    unsigned int vlan);
extern void
interface_unset_vlan(node_t *node,
                      interface_t *interface,
                      unsigned int vlan);

static int
intf_config_handler(param_t *param, ser_buff_t *tlv_buf, 
                    op_mode enable_or_disable){

   char *node_name;
   char *intf_name;
   unsigned int vlan_id;
   char *l2_mode_option;
   int CMDCODE;
   tlv_struct_t *tlv = NULL;
   node_t *node;
   interface_t *interface;

   CMDCODE = EXTRACT_CMD_CODE(tlv_buf);
   
    TLV_LOOP_BEGIN(tlv_buf, tlv){

        if     (strncmp(tlv->leaf_id, "node-name", strlen("node-name")) ==0)
            node_name = tlv->value;
        else if(strncmp(tlv->leaf_id, "if-name", strlen("if-name")) ==0)
            intf_name = tlv->value;
        else if(strncmp(tlv->leaf_id, "vlan-id", strlen("vlan-d")) ==0)
            vlan_id = atoi(tlv->value);
        else if(strncmp(tlv->leaf_id, "l2-mode-val", strlen("l2-mode-val")) == 0)
            l2_mode_option = tlv->value;
        else
            assert(0);
    } TLV_LOOP_END;

    node = get_node_by_node_name(topo, node_name);
    interface = get_node_if_by_name(node, intf_name);

    if(!interface){
        printf("Error : Interface %s do not exist\n", interface->if_name);
        return -1;
    }
    switch(CMDCODE){
        case CMDCODE_INTF_CONFIG_L2_MODE:
            switch(enable_or_disable){
                case CONFIG_ENABLE:
                    interface_set_l2_mode(node, interface, l2_mode_option);
                    break;
                case CONFIG_DISABLE:
                    interface_unset_l2_mode(node, interface, l2_mode_option);
                    break;
                default:
                    ;
            }
            break;
        case CMDCODE_INTF_CONFIG_VLAN:
            switch(enable_or_disable){
                case CONFIG_ENABLE:
                    interface_set_vlan(node, interface, vlan_id);
                    break;
                case CONFIG_DISABLE:
                    interface_unset_vlan(node, interface, vlan_id);
                    break;
                default:
                    ;
            }
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

         {
            /*show node*/    
             static param_t node;
             init_param(&node, CMD, "node", 0, 0, INVALID, 0, "\"node\" keyword");
             libcli_register_param(show, &node);
             libcli_register_display_callback(&node, dump_nw_node);
             {
                /*show node <node-name>*/ 
                 static param_t node_name;
                 init_param(&node_name, LEAF, 0, 0, validate_node_extistence, STRING, "node-name", "Node Name");
                 libcli_register_param(&node, &node_name);
                 {
                    /*show node <node-name> arp*/
                    static param_t arp;
                    init_param(&arp, CMD, "arp", show_arp_handler, 0, INVALID, 0, "Dump Arp Table");
                    libcli_register_param(&node_name, &arp);
                    set_param_cmd_code(&arp, CMDCODE_SHOW_NODE_ARP_TABLE);
                 }
                 {
                    /*show node <node-name> mac*/
                    static param_t mac;
                    init_param(&mac, CMD, "mac", show_mac_handler, 0, INVALID, 0, "Dump Mac table");
                    libcli_register_param(&node_name, &mac);
                    set_param_cmd_code(&mac, CMDCODE_SHOW_NODE_MAC_TABLE);
                 }
                  {
                    /*show node <node-name> rt*/
                    static param_t rt;
                    init_param(&rt, CMD, "rt", show_rt_handler, 0, INVALID, 0, "Dump L3 Routing table");
                    libcli_register_param(&node_name, &rt);
                    set_param_cmd_code(&rt, CMDCODE_SHOW_NODE_RT_TABLE);
                 }
                 {
                    /*show node <node-name> stp*/
                    static param_t stp;
                    init_param(&stp, CMD, "stp", show_stp_handler, 0, INVALID, 0, "Dump STP Status");
                    libcli_register_param(&node_name, &stp);
                    set_param_cmd_code(&stp, CMDCODE_SHOW_NODE_STP);
                 }
             }
         } 
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
             init_param(&node_name, LEAF, 0, 0,validate_node_extistence, STRING, "node-name", "Node Name");
             libcli_register_param(&node, &node_name);

             {
                /*run node <node-name> ping */
                static param_t ping;
                init_param(&ping, CMD, "ping" , 0, 0, INVALID, 0, "Ping utility");
                libcli_register_param(&node_name, &ping);
                {
                    /*run node <node-name> ping <ip-address>*/    
                    static param_t ip_addr;
                    init_param(&ip_addr, LEAF, 0, ping_handler, 0, IPV4, "ip-address", "Ipv4 Address");
                    libcli_register_param(&ping, &ip_addr);
                    set_param_cmd_code(&ip_addr, CMDCODE_PING);
                    {
                        static param_t ero;
                        init_param(&ero, CMD, "ero", 0, 0, INVALID, 0, "ERO(Explicit Route Object)");
                        libcli_register_param(&ip_addr, &ero);
                        {
                            static param_t ero_ip_addr;
                            init_param(&ero_ip_addr, LEAF, 0, ping_handler, 0, IPV4, "ero-ip-address", "ERO Ipv4 Address");
                            libcli_register_param(&ero, &ero_ip_addr);
                            set_param_cmd_code(&ero_ip_addr, CMDCODE_RUN_PING);
                        }
                    }
                }
             }

             { 
              /*run node <node-name> resolve-arp*/    
             static param_t resolve_arp;
              init_param(&resolve_arp, CMD, "resolve-arp", 0, 0, INVALID, 0, "Resolve ARP");
              libcli_register_param(&node_name, &resolve_arp);
             
              {
                  /*run node <node-name> resolve-arp <ip-address>*/    
                  static param_t ip_addr;
                  init_param(&ip_addr, LEAF, 0, arp_handler, 0, IPV4, "ip-address", "Nbr IPv4 Address");
                  libcli_register_param(&resolve_arp, &ip_addr);
                  set_param_cmd_code(&ip_addr, CMDCODE_RUN_ARP);
              }

            }

        }
     }
     
     {
        /* run find-paths */
        static param_t find_paths;
        init_param(&find_paths, CMD, "find-paths", 0, 0, INVALID, 0, "Find paths to node");
        libcli_register_param(run, &find_paths);
        {
            /* run find-paths <node-name> */
            static param_t node_name;
            init_param(&node_name, LEAF, 0, find_paths_handler, validate_node_extistence, STRING, "node-name", "Destination Node Name");
            libcli_register_param(&find_paths, &node_name);
            set_param_cmd_code(&node_name, CMDCODE_FIND_PATHS);
        }
     }
     
     {
        /* run arp-resolve-all */
        static param_t arp_resolve_all;
        init_param(&arp_resolve_all, CMD, "arp-resolve-all", arp_resolve_all_handler, 0, INVALID, 0, "Resolve ARP for all nodes");
        libcli_register_param(run, &arp_resolve_all);
        set_param_cmd_code(&arp_resolve_all, CMDCODE_RUN_ARP_RESOLVE_ALL);
     }
     
     {
        /* run route-configure-all */
        static param_t route_configure_all;
        init_param(&route_configure_all, CMD, "route-configure-all", route_configure_all_handler, 0, INVALID, 0, "Configure routes for all nodes");
        libcli_register_param(run, &route_configure_all);
        set_param_cmd_code(&route_configure_all, CMDCODE_ROUTE_CONFIGURE_ALL);
     }
     
     {
        /* man command */
        static param_t man;
        init_param(&man, CMD, "man", man_handler, 0, INVALID, 0, "Display manual pages for commands");
        libcli_register_param(root, &man);
        {
            static param_t cmd_name;
            init_param(&cmd_name, LEAF, 0, man_handler, 0, STRING, "command-name", "Command name");
            libcli_register_param(&man, &cmd_name);
        }
     }



      /*config node*/
    {
      static param_t node;
      init_param(&node, CMD, "node", 0, 0, INVALID, 0, "\"node\" keyword");
      libcli_register_param(config, &node);  
      libcli_register_display_callback(&node,dump_nw_node );
      {
        /*config node <node-name>*/
        static param_t node_name;
        init_param(&node_name, LEAF, 0, 0, validate_node_extistence, STRING, "node-name", "Node Name");
        libcli_register_param(&node, &node_name);
        {
            /*config node <node-name> interface*/
            static param_t interface;
            init_param(&interface, CMD, "interface", 0, 0, INVALID, 0, "\"interface\" keyword");    
            libcli_register_param(&node_name, &interface);
            {
                /*config node <node-name> interface <if-name>*/
                static param_t if_name;
                init_param(&if_name, LEAF, 0, 0, 0, STRING, "if-name", "Interface Name");
                libcli_register_param(&interface, &if_name);
                {
                    /*config node <node-name> interface <if-name> l2mode*/
                    static param_t l2_mode;
                    init_param(&l2_mode, CMD, "l2mode", 0, 0, INVALID, 0, "\"l2mode\" keyword");
                    libcli_register_param(&if_name, &l2_mode);
                    {
                        /*config node <node-name> interface <if-name> l2mode <access|trunk>*/
                        static param_t l2_mode_val;
                        init_param(&l2_mode_val, LEAF, 0, intf_config_handler, validate_l2_mode_value,  STRING, "l2-mode-val", "access|trunk");
                        libcli_register_param(&l2_mode, &l2_mode_val);
                        set_param_cmd_code(&l2_mode_val, CMDCODE_INTF_CONFIG_L2_MODE);
                    } 
                }
                {
                    /*config node <node-name> interface <if-name> vlan*/
                    static param_t vlan;
                    init_param(&vlan, CMD, "vlan", 0, 0, INVALID, 0, "\"vlan\" keyword");
                    libcli_register_param(&if_name, &vlan);
                    {
                        /*config node <node-name> interface <if-name> vlan <vlan-id>*/
                         static param_t vlan_id;
                         init_param(&vlan_id, LEAF, 0, intf_config_handler, validate_vlan_id, INT, "vlan-id", "vlan id(1-4096)");
                         libcli_register_param(&vlan, &vlan_id);
                         set_param_cmd_code(&vlan_id, CMDCODE_INTF_CONFIG_VLAN);
                    }   
                }    
            }
            
        }
        
        {
            /*config node <node-name> route*/
            static param_t route;
            init_param(&route, CMD, "route", 0, 0, INVALID, 0, "L3 route");
            libcli_register_param(&node_name, &route);
            {
                /*config node <node-name> route <ip-address>*/    
                static param_t ip_addr;
                init_param(&ip_addr, LEAF, 0, 0, 0, IPV4, "ip-address", "IPv4 Address");
                libcli_register_param(&route, &ip_addr);
                {
                     /*config node <node-name> route <ip-address> <mask>*/
                    static param_t mask;
                    init_param(&mask, LEAF, 0, l3_config_handler, validate_mask_value, INT, "mask", "mask(0-32");
                    libcli_register_param(&ip_addr, &mask);
                    set_param_cmd_code(&mask, CMDCODE_CONF_NODE_L3ROUTE);
                    {
                        /*config node <node-name> route <ip-address> <mask> <gw-ip>*/
                        static param_t gwip;
                        init_param(&gwip, LEAF, 0, 0, 0, IPV4, "gw-ip", "IPv4 Address");
                        libcli_register_param(&mask, &gwip);
                        {
                            /*config node <node-name> route <ip-address> <mask> <gw-ip> <oif>*/
                            static param_t oif;
                            init_param(&oif, LEAF, 0, l3_config_handler, 0, STRING, "oif", "Out-going intf Name");
                            libcli_register_param(&gwip, &oif);
                            set_param_cmd_code(&oif, CMDCODE_CONF_NODE_L3ROUTE);
                        }
                    }
                }
            }    
        }    
        support_cmd_negation(&node_name);
      }
    }
        support_cmd_negation(config);
     
}

