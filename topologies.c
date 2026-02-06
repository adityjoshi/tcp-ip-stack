#include "gluethread/glthread.h"
#include "graph.h"
#include "communication.h"
#include "layer2/layer2.h"
#include <unistd.h>
extern void
network_start_packet_receiver_thread(graph_t *topo);

extern void node_set_intf_l2_mode(node_t *node, char *intf_name, intf_l2_mode_t intf_l2_mode);
extern void node_set_intf_vlan_membership(node_t *node, char *intf_name, unsigned int vlan_id);
extern void stp_start(node_t *node);

graph_t * build_first_topo() {

   
#if 0

                          +----------+
                      0/4 |          |0/0
         +----------------+   R0_re  +---------------------------+
         |     40.1.1.1/24| 122.1.1.0|20.1.1.1/24                |
         |                +----------+                           |
         |                                                       |
         |                                                       |
         |                                                       |
         |40.1.1.2/24                                            |20.1.1.2/24
         |0/5                                                    |0/1
     +---+---+                                              +----+-----+
     |       |0/3                                        0/2|          |
     | R2_re +----------------------------------------------+    R1_re | 
     |       |30.1.1.2/24                        30.1.1.1/24|          |
     +-------+                                              +----------+
#endif /* if 0 */



    graph_t *topo = create_new_graph("Hello World Generic Graph");
    node_t *R0_re = create_new_node(topo, "R0_re");
    node_t *R1_re = create_new_node(topo, "R1_re");
    node_t *R2_re = create_new_node(topo, "R2_re");

    insert_link_between_node(R0_re, R1_re, "eth0/0", "eth0/1", 1);
    insert_link_between_node(R1_re, R2_re, "eth0/2", "eth0/3", 1);
    

    node_set_loopback_address(R0_re, "122.1.1.0");
    node_set_interface_ip_address(R0_re, "eth0/0", "20.1.1.1", 24);
   
    
    
    node_set_loopback_address(R1_re, "122.1.1.1");
    node_set_interface_ip_address(R1_re, "eth0/1", "20.1.1.2", 24);
    node_set_interface_ip_address(R1_re, "eth0/2", "30.1.1.1", 24);
    
    node_set_loopback_address(R2_re, "122.1.1.2");
    node_set_interface_ip_address(R2_re, "eth0/3", "30.1.1.2", 24);

    network_start_packet_receiver_thread(topo);
    return topo;
}

graph_t * build_linear_topo() {

   
#if 0

                          +----------+
                      0/4 |          |0/0
         +----------------+   R0_re  +---------------------------+
         |     40.1.1.1/24| 122.1.1.0|20.1.1.1/24                |
         |                +----------+                           |
         |                                                       |
         |                                                       |
         |                                                       |
         |40.1.1.2/24                                            |20.1.1.2/24
         |0/5                                                    |0/1
     +---+---+                                              +----+-----+
     |       |0/3                                        0/2|          |
     | R2_re +----------------------------------------------+    R1_re | 
     |       |30.1.1.2/24                        30.1.1.1/24|          |
     +-------+                                              +----------+
#endif /* if 0 */



    graph_t *topo = create_new_graph("Linear Topology");
    node_t *R0_re = create_new_node(topo, "R0_re");
    node_t *R1_re = create_new_node(topo, "R1_re");
    node_t *R2_re = create_new_node(topo, "R2_re");

    insert_link_between_node(R0_re, R1_re, "eth0/0", "eth0/1", 1);
    insert_link_between_node(R1_re, R2_re, "eth0/2", "eth0/3", 1);
    

    node_set_loopback_address(R0_re, "122.1.1.0");
    node_set_interface_ip_address(R0_re, "eth0/0", "20.1.1.1", 24);
   
    
    
    node_set_loopback_address(R1_re, "122.1.1.1");
    node_set_interface_ip_address(R1_re, "eth0/1", "20.1.1.2", 24);
    node_set_interface_ip_address(R1_re, "eth0/2", "30.1.1.1", 24);
    
    node_set_loopback_address(R2_re, "122.1.1.2");
    node_set_interface_ip_address(R2_re, "eth0/3", "30.1.1.2", 24);

    network_start_packet_receiver_thread(topo);
    return topo;
}

graph_t *build_dualswitch_topo(){

#if 0

                    +-------+                                    +-------+
                    | L2SW1 |                                    | L2SW2 |
                    |       |                                    |       |
                    +---+---+                                    +---+---+
                        |                                            |
                        |                                            |
                        |                                            |
                    +---+---+                                    +---+---+
                    | H1   |                                    | H3   |
                    |      |                                    |      |
                    +------+                                    +------+
                    +------+                                    +------+
                    | H2   |                                    | H4   |
                    |      |                                    |      |
                    +------+                                    +------+
#endif

    graph_t *topo = create_new_graph("Dual Switch Topology");
    node_t *H1 = create_new_node(topo, "H1");
    node_t *H2 = create_new_node(topo, "H2");
    node_t *H3 = create_new_node(topo, "H3");
    node_t *H4 = create_new_node(topo, "H4");
    node_t *L2SW1 = create_new_node(topo, "L2SW1");
    node_t *L2SW2 = create_new_node(topo, "L2SW2");

    insert_link_between_node(H1, L2SW1, "eth0/1", "eth0/1", 1);
    insert_link_between_node(H2, L2SW1, "eth0/1", "eth0/2", 1);
    insert_link_between_node(H3, L2SW2, "eth0/1", "eth0/1", 1);
    insert_link_between_node(H4, L2SW2, "eth0/1", "eth0/2", 1);
    insert_link_between_node(L2SW1, L2SW2, "eth0/3", "eth0/3", 1);

    node_set_interface_ip_address(H1, "eth0/1", "10.1.1.1", 24);
    node_set_interface_ip_address(H2, "eth0/1", "10.1.1.2", 24);
    node_set_interface_ip_address(H3, "eth0/1", "10.1.1.3", 24);
    node_set_interface_ip_address(H4, "eth0/1", "10.1.1.4", 24);

    node_set_intf_l2_mode(L2SW1, "eth0/1", ACCESS);
    node_set_intf_vlan_membership(L2SW1, "eth0/1", 10);
    node_set_intf_l2_mode(L2SW1, "eth0/2", ACCESS);
    node_set_intf_vlan_membership(L2SW1, "eth0/2", 10);
    node_set_intf_l2_mode(L2SW1, "eth0/3", TRUNK);
    node_set_intf_vlan_membership(L2SW1, "eth0/3", 10);

    node_set_intf_l2_mode(L2SW2, "eth0/1", ACCESS);
    node_set_intf_vlan_membership(L2SW2, "eth0/1", 10);
    node_set_intf_l2_mode(L2SW2, "eth0/2", ACCESS);
    node_set_intf_vlan_membership(L2SW2, "eth0/2", 10);
    node_set_intf_l2_mode(L2SW2, "eth0/3", TRUNK);
    node_set_intf_vlan_membership(L2SW2, "eth0/3", 10);

    network_start_packet_receiver_thread(topo);
    return topo;
}

graph_t *build_simple_l2_switch_topo() {

    graph_t *topo = create_new_graph("Simple L2 Switch Topology");
    node_t *H1 = create_new_node(topo, "H1");
    node_t *H2 = create_new_node(topo, "H2");
    node_t *H3 = create_new_node(topo, "H3");
    node_t *H4 = create_new_node(topo, "H4");
    node_t *L2SW = create_new_node(topo, "L2SW");

    insert_link_between_node(H1, L2SW, "eth0/5", "eth0/4", 1);
    insert_link_between_node(H2, L2SW, "eth0/8", "eth0/3", 1);
    insert_link_between_node(H3, L2SW, "eth0/6", "eth0/2", 1);
    insert_link_between_node(H4, L2SW, "eth0/7", "eth0/1", 1);

    node_set_interface_ip_address(H1, "eth0/5", "10.1.1.2", 24);
    node_set_interface_ip_address(H2, "eth0/8", "10.1.1.4", 24);
    node_set_interface_ip_address(H3, "eth0/6", "10.1.1.1", 24);
    node_set_interface_ip_address(H4, "eth0/7", "10.1.1.3", 24);
    node_set_intf_l2_mode(L2SW, "eth0/4", ACCESS);
    node_set_intf_l2_mode(L2SW, "eth0/3", ACCESS);
    node_set_intf_l2_mode(L2SW, "eth0/2", ACCESS);
    node_set_intf_l2_mode(L2SW, "eth0/1", ACCESS);

    network_start_packet_receiver_thread(topo);
    return topo;
}

graph_t *L2_loop_topo(){

#if 0

                    +-------+                                    +-------+
                    | L2SW1 |                                    | L2SW2 |
                    |       |                                    |       |
                    +---+---+                                    +---+---+
                        |                                            |
                        |                                            |
                        |                                            |
                    +---+---+                                    +---+---+
                    | H1   |                                    | H3   |
                    |      |                                    |      |
                    +------+                                    +------+
                    +------+                                    +------+
                    | H2   |                                    | H4   |
                    |      |                                    |      |
                    +------+                                    +------+
#endif

    graph_t *topo = create_new_graph("L2 Loop Topology");
    node_t *H1 = create_new_node(topo, "H1");
    node_t *H6 = create_new_node(topo, "H6");
    node_set_loopback_address(H1, "122.1.1.1");
    node_set_loopback_address(H6, "122.1.1.6");
    
    node_t *L2SW1 = create_new_node(topo, "L2SW1");
    node_t *L2SW2 = create_new_node(topo, "L2SW2");
    node_t *L2SW3 = create_new_node(topo, "L2SW3");
    node_t *L2SW4 = create_new_node(topo, "L2SW4");

    insert_link_between_node(H1, L2SW1, "eth0/1", "eth0/2", 1);
    insert_link_between_node(L2SW1, L2SW2, "eth0/5", "eth0/7", 1);
    insert_link_between_node(L2SW2, L2SW3, "eth0/9", "eth0/8", 1);
    insert_link_between_node(L2SW3, L2SW4, "eth0/4", "eth0/4", 1);
    insert_link_between_node(L2SW4, L2SW1, "eth0/3", "eth0/7", 1);
    insert_link_between_node(L2SW2, H6, "eth0/10", "eth0/11", 1);
    
    node_set_interface_ip_address(H1, "eth0/1", "10.1.1.1", 24);
    node_set_interface_ip_address(H6, "eth0/11", "10.1.1.6", 24);

    node_set_intf_l2_mode(L2SW1, "eth0/2", ACCESS);
    node_set_intf_vlan_membership(L2SW1, "eth0/2", 10);
    node_set_intf_l2_mode(L2SW1, "eth0/7", TRUNK);
    node_set_intf_vlan_membership(L2SW1, "eth0/7", 10);
    node_set_intf_l2_mode(L2SW1, "eth0/5", TRUNK);
    node_set_intf_vlan_membership(L2SW1, "eth0/5", 10);

    node_set_intf_l2_mode(L2SW2, "eth0/7", TRUNK);
    node_set_intf_vlan_membership(L2SW2, "eth0/7", 10);
    node_set_intf_l2_mode(L2SW2, "eth0/10", ACCESS);
    node_set_intf_vlan_membership(L2SW2, "eth0/10", 10);
    node_set_intf_l2_mode(L2SW2, "eth0/9", TRUNK);
    node_set_intf_vlan_membership(L2SW2, "eth0/9", 10);

    node_set_intf_l2_mode(L2SW3, "eth0/4", TRUNK);
    node_set_intf_vlan_membership(L2SW3, "eth0/4", 10);
    node_set_intf_l2_mode(L2SW3, "eth0/8", TRUNK);
    node_set_intf_vlan_membership(L2SW3, "eth0/8", 10);

    node_set_intf_l2_mode(L2SW4, "eth0/4", TRUNK);
    node_set_intf_vlan_membership(L2SW4, "eth0/4", 10);
    node_set_intf_l2_mode(L2SW4, "eth0/3", TRUNK);
    node_set_intf_vlan_membership(L2SW4, "eth0/3", 10);

    network_start_packet_receiver_thread(topo);
    return topo;
}




graph_t *
linear_3_node_topo(){

#if 0

                                        +---------|                                  +----------+
+--------+                              |         |                                  |R3        |
|R1      |eth0/1                  eth0/2|R2       |eth0/3                      eth0/4|122.1.1.3 |
|122.1.1.1+-----------------------------+122.1.1.2|+----------------------------------+         |
|        |10.1.1.1/24        10.1.1.2/24|         |11.1.1.2/24            11.1.1.1/24|          |
+--------+                              +-------+-|                                  +----------+

#endif


    graph_t *topo = create_new_graph("3 node linerar topo");
    node_t *R1 = create_new_node(topo, "R1");
    node_t *R2 = create_new_node(topo, "R2");
    node_t *R3 = create_new_node(topo, "R3");

    insert_link_between_node(R1, R2, "eth0/1", "eth0/2", 1);
    insert_link_between_node(R2, R3, "eth0/3", "eth0/4", 1);

    node_set_loopback_address(R1, "122.1.1.1");
    node_set_interface_ip_address(R1, "eth0/1", "10.1.1.1", 24);
    
    node_set_loopback_address(R2, "122.1.1.2");
    node_set_interface_ip_address(R2, "eth0/2", "10.1.1.2", 24);
    node_set_interface_ip_address(R2, "eth0/3", "11.1.1.2", 24);

    node_set_loopback_address(R3, "122.1.1.3");
    node_set_interface_ip_address(R3, "eth0/4", "11.1.1.1", 24);

    network_start_packet_receiver_thread(topo);

    return topo;
}

/*
 * Loop Topology - Creates routing loops for testing
 * 
 * Triangle topology:
 *     R1 (10.1.1.1) ---- R2 (20.1.1.1)
 *      |                    |
 *      |                    |
 *     R3 (30.1.1.1) --------
 * 
 * With misconfigured routes, packets will loop:
 * R1 -> R2 -> R3 -> R1 -> R2 -> ... (until TTL expires)
 */
graph_t *build_loop_topo() {
    graph_t *topo = create_new_graph("Loop Test Topology");
    
    node_t *R1 = create_new_node(topo, "R1");
    node_t *R2 = create_new_node(topo, "R2");
    node_t *R3 = create_new_node(topo, "R3");
    
    // Create triangle: R1-R2, R2-R3, R3-R1
    insert_link_between_node(R1, R2, "eth0/1", "eth0/1", 1);
    insert_link_between_node(R2, R3, "eth0/2", "eth0/1", 1);
    insert_link_between_node(R3, R1, "eth0/2", "eth0/2", 1);
    
    // Configure IP addresses
    node_set_loopback_address(R1, "122.1.1.1");
    node_set_interface_ip_address(R1, "eth0/1", "10.1.1.1", 24);
    node_set_interface_ip_address(R1, "eth0/2", "30.1.1.2", 24);
    
    node_set_loopback_address(R2, "122.1.1.2");
    node_set_interface_ip_address(R2, "eth0/1", "10.1.1.2", 24);
    node_set_interface_ip_address(R2, "eth0/2", "20.1.1.1", 24);
    
    node_set_loopback_address(R3, "122.1.1.3");
    node_set_interface_ip_address(R3, "eth0/1", "20.1.1.2", 24);
    node_set_interface_ip_address(R3, "eth0/2", "30.1.1.1", 24);
    
    network_start_packet_receiver_thread(topo);
    
    return topo;
}

/*
 * STP Loop Topology - Demonstrates STP preventing loops
 * 
 * Triangle topology with 3 switches forming a loop:
 *     SW1 ---- SW2
 *      |        |
 *      |        |
 *     SW3 --------
 * 
 * STP will block one port to break the loop
 */
graph_t *build_stp_loop_topo() {
    graph_t *topo = create_new_graph("STP Loop Test Topology");
    
    node_t *SW1 = create_new_node(topo, "SW1");
    node_t *SW2 = create_new_node(topo, "SW2");
    node_t *SW3 = create_new_node(topo, "SW3");
    node_t *H1 = create_new_node(topo, "H1");
    node_t *H2 = create_new_node(topo, "H2");
    
    // Create triangle loop: SW1-SW2, SW2-SW3, SW3-SW1
    insert_link_between_node(SW1, SW2, "eth0/1", "eth0/1", 1);
    insert_link_between_node(SW2, SW3, "eth0/2", "eth0/1", 1);
    insert_link_between_node(SW3, SW1, "eth0/2", "eth0/2", 1);
    
    // Connect hosts
    insert_link_between_node(H1, SW1, "eth0/1", "eth0/3", 1);
    insert_link_between_node(H2, SW3, "eth0/1", "eth0/3", 1);
    
    // Configure IP addresses on hosts
    node_set_interface_ip_address(H1, "eth0/1", "10.1.1.1", 24);
    node_set_interface_ip_address(H2, "eth0/1", "10.1.1.2", 24);
    
    // Configure L2 mode on switches
    node_set_intf_l2_mode(SW1, "eth0/1", TRUNK);
    node_set_intf_vlan_membership(SW1, "eth0/1", 10);
    node_set_intf_l2_mode(SW1, "eth0/2", TRUNK);
    node_set_intf_vlan_membership(SW1, "eth0/2", 10);
    node_set_intf_l2_mode(SW1, "eth0/3", ACCESS);
    node_set_intf_vlan_membership(SW1, "eth0/3", 10);
    
    node_set_intf_l2_mode(SW2, "eth0/1", TRUNK);
    node_set_intf_vlan_membership(SW2, "eth0/1", 10);
    node_set_intf_l2_mode(SW2, "eth0/2", TRUNK);
    node_set_intf_vlan_membership(SW2, "eth0/2", 10);
    
    node_set_intf_l2_mode(SW3, "eth0/1", TRUNK);
    node_set_intf_vlan_membership(SW3, "eth0/1", 10);
    node_set_intf_l2_mode(SW3, "eth0/2", TRUNK);
    node_set_intf_vlan_membership(SW3, "eth0/2", 10);
    node_set_intf_l2_mode(SW3, "eth0/3", ACCESS);
    node_set_intf_vlan_membership(SW3, "eth0/3", 10);
    
    network_start_packet_receiver_thread(topo);
    
    // Initialize and start STP on all switches (after network thread starts)
    // This allows BPDUs to be processed
    // Use shorter delays to avoid blocking startup
    stp_start(SW1);
    usleep(50000); // Small delay for BPDU processing
    stp_start(SW2);
    usleep(50000);
    stp_start(SW3);
    usleep(100000); // Allow time for initial BPDU exchange
    
    return topo;
}

// Topology with L2 loop - NO automatic STP blocking or pathfinder
// This is used to demonstrate the infinite loop problem
// Solution: Run "run find-paths <node-name>" to block redundant ports
graph_t *build_loop_demo_topo() {
    /*
     * Topology with L2 loop for demonstration:
     * 
     *     H1 (10.1.1.1)          H2 (10.1.1.2)
     *        |                      |
     *        |                      |
     *     eth0/1                 eth0/1
     *        |                      |
     *        +----+            +----+
     *             |            |
     *          eth0/1       eth0/3
     *             |            |
     *          +--+--+      +--+--+
     *          | SW1 |------| SW3 |
     *          +--+--+      +--+--+
     *             |            |
     *          eth0/2       eth0/2
     *             |            |
     *          +--+--+      +--+--+
     *          | SW2 +------+     |
     *          +-----+      +-----+
     * 
     * All switches connected in triangle - creates L2 loop
     * No STP blocking initially - will cause infinite loop
     */
    
    graph_t *topo = create_new_graph("Loop Demo Topology - No Auto Blocking");
    
    // Create nodes
    node_t *H1 = create_new_node(topo, "H1");
    node_t *H2 = create_new_node(topo, "H2");
    node_t *SW1 = create_new_node(topo, "SW1");
    node_t *SW2 = create_new_node(topo, "SW2");
    node_t *SW3 = create_new_node(topo, "SW3");
    
    // Create links - triangle loop between switches
    insert_link_between_node(H1, SW1, "eth0/1", "eth0/3", 1);
    insert_link_between_node(H2, SW3, "eth0/1", "eth0/3", 1);
    insert_link_between_node(SW1, SW2, "eth0/1", "eth0/1", 1);
    insert_link_between_node(SW2, SW3, "eth0/2", "eth0/1", 1);
    insert_link_between_node(SW3, SW1, "eth0/2", "eth0/2", 1);
    
    // Configure IP addresses on hosts
    node_set_loopback_address(H1, "1.1.1.1");
    node_set_interface_ip_address(H1, "eth0/1", "10.1.1.1", 24);
    
    node_set_loopback_address(H2, "2.2.2.2");
    node_set_interface_ip_address(H2, "eth0/1", "10.1.1.2", 24);
    
    // Configure switches as L2 switches (TRUNK mode for inter-switch links, ACCESS for host links)
    node_set_intf_l2_mode(SW1, "eth0/1", TRUNK);   // Inter-switch
    node_set_intf_l2_mode(SW1, "eth0/2", TRUNK);   // Inter-switch
    node_set_intf_l2_mode(SW1, "eth0/3", ACCESS);  // Host link
    
    node_set_intf_l2_mode(SW2, "eth0/1", TRUNK);   // Inter-switch
    node_set_intf_l2_mode(SW2, "eth0/2", TRUNK);   // Inter-switch
    
    node_set_intf_l2_mode(SW3, "eth0/1", TRUNK);   // Inter-switch
    node_set_intf_l2_mode(SW3, "eth0/2", TRUNK);   // Inter-switch
    node_set_intf_l2_mode(SW3, "eth0/3", ACCESS);  // Host link
    
    // Set VLAN membership for ACCESS ports
    node_set_intf_vlan_membership(SW1, "eth0/3", 10);
    node_set_intf_vlan_membership(SW3, "eth0/3", 10);
    
    // Set VLAN membership for TRUNK ports (optional, but good practice)
    node_set_intf_vlan_membership(SW1, "eth0/1", 10);
    node_set_intf_vlan_membership(SW1, "eth0/2", 10);
    node_set_intf_vlan_membership(SW2, "eth0/1", 10);
    node_set_intf_vlan_membership(SW2, "eth0/2", 10);
    node_set_intf_vlan_membership(SW3, "eth0/1", 10);
    node_set_intf_vlan_membership(SW3, "eth0/2", 10);
    
    network_start_packet_receiver_thread(topo);
    
    // NOTE: We do NOT initialize STP or run pathfinder here
    // This allows the user to demonstrate the infinite loop problem
    // Then they can run "run find-paths H2" to see the solution
    
    return topo;
}

// Topology for testing route-configure-all command
// Multiple routers on different subnets to test automatic route configuration
graph_t *build_route_test_topo() {
    /*
     * Topology for route-configure-all testing:
     * 
     *     R1 (10.1.1.1/24)          R2 (20.1.1.1/24)
     *        |                          |
     *        |eth0/1                eth0/1|
     *        |                          |
     *        +----------+----------+-----+
     *                   |          |
     *                eth0/2     eth0/2
     *                   |          |
     *                +--+--+      +--+--+
     *                | R3  |------| R4  |
     *                +--+--+      +--+--+
     *                   |          |
     *                eth0/3     eth0/3
     *                   |          |
     *                +--+--+      +--+--+
     *                | H1  |      | H2  |
     *                +-----+      +-----+
     *              (30.1.1.10)   (40.1.1.10)
     * 
     * Subnets:
     * - R1-R3: 10.1.1.0/24 (R1: 10.1.1.1, R3: 10.1.1.2)
     * - R2-R4: 20.1.1.0/24 (R2: 20.1.1.1, R4: 20.1.1.2)
     * - R3-R4: 30.1.1.0/24 (R3: 30.1.1.1, R4: 30.1.1.2)
     * - R3-H1: 30.1.1.0/24 (R3: 30.1.1.2, H1: 30.1.1.10)
     * - R4-H2: 40.1.1.0/24 (R4: 40.1.1.1, H2: 40.1.1.10)
     * 
     * This topology allows testing route-configure-all to add routes
     * between different subnets.
     */
    
    graph_t *topo = create_new_graph("Route Configuration Test Topology");
    
    // Create nodes
    node_t *R1 = create_new_node(topo, "R1");
    node_t *R2 = create_new_node(topo, "R2");
    node_t *R3 = create_new_node(topo, "R3");
    node_t *R4 = create_new_node(topo, "R4");
    node_t *H1 = create_new_node(topo, "H1");
    node_t *H2 = create_new_node(topo, "H2");
    
    // Create links
    insert_link_between_node(R1, R3, "eth0/1", "eth0/1", 1);
    insert_link_between_node(R2, R4, "eth0/1", "eth0/1", 1);
    insert_link_between_node(R3, R4, "eth0/2", "eth0/2", 1);
    insert_link_between_node(R3, H1, "eth0/3", "eth0/1", 1);
    insert_link_between_node(R4, H2, "eth0/3", "eth0/1", 1);
    
    // Configure IP addresses
    node_set_loopback_address(R1, "1.1.1.1");
    node_set_interface_ip_address(R1, "eth0/1", "10.1.1.1", 24);
    
    node_set_loopback_address(R2, "2.2.2.2");
    node_set_interface_ip_address(R2, "eth0/1", "20.1.1.1", 24);
    
    node_set_loopback_address(R3, "3.3.3.3");
    node_set_interface_ip_address(R3, "eth0/1", "10.1.1.2", 24);  // Connected to R1
    node_set_interface_ip_address(R3, "eth0/2", "30.1.1.1", 24);  // Connected to R4
    node_set_interface_ip_address(R3, "eth0/3", "30.1.1.2", 24);  // Connected to H1 (same subnet as R4 link)
    
    node_set_loopback_address(R4, "4.4.4.4");
    node_set_interface_ip_address(R4, "eth0/1", "20.1.1.2", 24);  // Connected to R2
    node_set_interface_ip_address(R4, "eth0/2", "30.1.1.3", 24);  // Connected to R3
    node_set_interface_ip_address(R4, "eth0/3", "40.1.1.1", 24);  // Connected to H2
    
    node_set_loopback_address(H1, "10.10.10.10");
    node_set_interface_ip_address(H1, "eth0/1", "30.1.1.10", 24);
    
    node_set_loopback_address(H2, "20.20.20.20");
    node_set_interface_ip_address(H2, "eth0/1", "40.1.1.10", 24);
    
    network_start_packet_receiver_thread(topo);
    
    return topo;
}
