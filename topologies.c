#include "gluethread/glthread.h"
#include "graph.h"
#include "communication.h"
#include "layer2/layer2.h"
extern void
network_start_packet_receiver_thread(graph_t *topo);

extern void node_set_intf_l2_mode(node_t *node, char *intf_name, intf_l2_mode_t intf_l2_mode);

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

       +----------+                      +----------+                      +----------+
       |          |0/1                0/2|          |0/3        20.1.1.2/24 |           |
 ------+   R0_re  +----------------------+   R1_re  +----------------------+  R2_re    +
       | 122.1.1.1|10.1.1.1/24           |122.1.1.2 |20.1.1.2/24        0/4|122.1.1.3  |
       +----------+           10.1.1.2/24+----------+                      +-----------+




#endif /* if 0 */
graph_t *topo = create_new_graph("3 Node linear topo");
node_t *R0_re = create_new_node(topo, "R0_re");
node_t *R1_re = create_new_node(topo, "R1_re");
node_t *R2_re = create_new_node(topo, "R2_re");

node_set_loopback_address(R0_re, "122.1.1.1");
node_set_loopback_address(R1_re, "122.1.1.2");
node_set_loopback_address(R2_re, "122.1.1.3");

insert_link_between_node(R0_re, R1_re, "eth0/1", "eth0/2", 1);
insert_link_between_node(R1_re, R2_re, "eth0/3", "eth0/4", 1);

node_set_interface_ip_address(R0_re, "eth0/1", "10.1.1.1",24);
node_set_interface_ip_address(R1_re, "eth0/2", "10.1.1.2",24);
node_set_interface_ip_address(R1_re, "eth0/3", "20.1.1.1",24);
node_set_interface_ip_address(R2_re, "eth0/4", "20.1.1.2",24);

return topo;

}



graph_t *
build_simple_l2_switch_topo(){

#if 0             
                                       +-----------+
                                       |  H4       |
                                       | 122.1.1.4 |
                                       +----+------+
                                            |eth0/7 - 10.1.1.3/24       
                                            |       
                                            |eth0/1
                                       +----+----+                        +---------+
       +---------+                     |         |                        |         |
       |         |10.1.1.2/24          |   L2Sw  |eth0/2       10.1.1.1/24|  H3     |
       |  H1     +---------------------+         +------------------------+122.1.1.3|
       |122.1.1.1|eth0/5         eth0/4|         |                 eth0/6 |         |
       + --------+                     |         |                        |         |
                                       +----+----+                        +---------+
                                            |eth0/3     
                                            |
                                            |
                                            |
                                            |10.1.1.4/24
                                            |eth0/8
                                      +----++------+
                                      |            |
                                      |   H2       |
                                      |122.1.1.2   |
                                      |            |
                                      +------------+

#endif


graph_t *graph = create_new_graph("L2 switch topology");
node_t *H1 = create_new_node(graph, "H1");
node_t *H2 = create_new_node(graph, "H2");  
node_t *H3 = create_new_node(graph, "H3");
node_t *H4 = create_new_node(graph, "H4");
node_t *L2Sw = create_new_node(graph, "L2Sw");

insert_link_between_node(H1, L2Sw, "eth0/5", "eth0/4", 1);
insert_link_between_node(H2, L2Sw, "eth0/8", "eth0/3", 1);
insert_link_between_node(H3, L2Sw, "eth0/6", "eth0/2", 1);
insert_link_between_node(H4, L2Sw, "eth0/7", "eth0/1", 1);


node_set_loopback_address(H1, "122.1.1.1");
node_set_interface_ip_address(H1, "eth0/5", "10.1.1.2", 24);


node_set_loopback_address(H2, "122.1.1.2");
node_set_interface_ip_address(H2, "eth0/8", "10.1.1.4", 24);


node_set_loopback_address(H3, "122.1.1.3");
node_set_interface_ip_address(H3, "eth0/6", "10.1.1.1", 24);


node_set_loopback_address(H4, "122.1.1.4");
node_set_interface_ip_address(H4, "eth0/7", "10.1.1.3", 24);


node_set_intf_l2_mode(L2Sw, "eth0/1", ACCESS);
node_set_intf_l2_mode(L2Sw, "eth0/2", ACCESS);
node_set_intf_l2_mode(L2Sw, "eth0/3", ACCESS);  
node_set_intf_l2_mode(L2Sw, "eth0/4", ACCESS);

network_start_packet_receiver_thread(graph);
return graph; 

}








graph_t *build_dualswitch_topo(){

#if 0
                                    +---------+                               +----------+
                                    |         |                               |          |
                                    |  H2     |                               |  H5      |
                                    |122.1.1.2|                               |122.1.1.5 |                                           
                                    +---+-----+                               +-----+----+                                           
                                        |10.1.1.2/24                                +10.1.1.5/24                                                
                                        |eth0/3                                     |eth0/8                                                
                                        |                                           |                                                
                                        |eth0/7,AC,V10                              |eth0/9,AC,V10                                                
                                  +-----+----+                                +-----+---+                                            
                                  |          |                                |         |                                            
   +------+---+                   |          |                                |         |                         +--------+         
   |  H1      |10.1.1.1/24        |   L2SW1  |eth0/5                    eth0/7| L2SW2   |eth0/10           eth0/13|  H6    |         
   |122.1.1.1 +-------------------|          |+-------------------------------|         +-------------+----------+122.1.1.6|         
   +------+---+ eth0/1      eth0/2|          |TR,V10,V11            TR,V10,V11|         |AC,V10        10.1.1.6/24|        |         
                            AC,V10|          |                                |         |                         +-+------+         
                                  +-----+----+                                +----+----+                                            
                                        |eth0/6                                    |eth0/12     
                                        |AC,V11                                    |AC,V11 
                                        |                                          |  
                                        |                                          |  
                                        |                                          |  
                                        |                                          |eth0/11
                                        |eth0/4                                    |10.1.1.4/24  
                                        |10.1.1.3/24                             +--+-----+
                                   +----+---+|                                   | H4     |
                                   |  H3     |                                   |        |
                                   |122.1.1.3|                                   |122.1.1.4|
                                   +--------+|                                   +--------+
#endif

    graph_t *topo = create_new_graph("Dual Switch Topo");
    node_t *H1 = create_new_node(topo, "H1");
    node_set_loopback_address(H1, "122.1.1.1");
    node_t *H2 = create_new_node(topo, "H2");
    node_set_loopback_address(H2, "122.1.1.2");
    node_t *H3 = create_new_node(topo, "H3");
    node_set_loopback_address(H3, "122.1.1.3");
    node_t *H4 = create_new_node(topo, "H4");
    node_set_loopback_address(H4, "122.1.1.4");
    node_t *H5 = create_new_node(topo, "H5");

    node_set_loopback_address(H5, "122.1.1.5");
    node_t *H6 = create_new_node(topo, "H6");
    node_set_loopback_address(H6, "122.1.1.6");
    node_t *L2SW1 = create_new_node(topo, "L2SW1");
    node_t *L2SW2 = create_new_node(topo, "L2SW2");
    
    insert_link_between_node(H1, L2SW1, "eth0/1", "eth0/2", 1);
    insert_link_between_node(H2, L2SW1, "eth0/3", "eth0/7", 1);
    insert_link_between_node(H3, L2SW1, "eth0/4", "eth0/6", 1);
    insert_link_between_node(L2SW1, L2SW2, "eth0/5", "eth0/7", 1);
    insert_link_between_node(H5, L2SW2, "eth0/8", "eth0/9", 1);
    insert_link_between_node(H4, L2SW2, "eth0/11", "eth0/12", 1);
    insert_link_between_node(H6, L2SW2, "eth0/13", "eth0/10", 1);

    node_set_interface_ip_address(H1, "eth0/1",  "10.1.1.1", 24);
    node_set_interface_ip_address(H2, "eth0/3",  "10.1.1.2", 24);
    node_set_interface_ip_address(H3, "eth0/4",  "10.1.1.3", 24);
    node_set_interface_ip_address(H4, "eth0/11", "10.1.1.4", 24);
    node_set_interface_ip_address(H5, "eth0/8",  "10.1.1.5", 24);
    node_set_interface_ip_address(H6, "eth0/13", "10.1.1.6", 24);

    node_set_intf_l2_mode(L2SW1, "eth0/2", ACCESS);
    node_set_intf_vlan_membership(L2SW1, "eth0/2", 10);
    node_set_intf_l2_mode(L2SW1, "eth0/7", ACCESS);
    node_set_intf_vlan_membership(L2SW1, "eth0/7", 10);
    node_set_intf_l2_mode(L2SW1, "eth0/5", TRUNK);
    node_set_intf_vlan_membership(L2SW1, "eth0/5", 10);
    node_set_intf_vlan_membership(L2SW1, "eth0/5", 11);
    node_set_intf_l2_mode(L2SW1, "eth0/6", ACCESS);
    node_set_intf_vlan_membership(L2SW1, "eth0/6", 11);

    node_set_intf_l2_mode(L2SW2, "eth0/7", TRUNK);
    node_set_intf_vlan_membership(L2SW2, "eth0/7", 10);
    node_set_intf_vlan_membership(L2SW2, "eth0/7", 11);
    node_set_intf_l2_mode(L2SW2, "eth0/9", ACCESS);
    node_set_intf_vlan_membership(L2SW2, "eth0/9", 10);
    node_set_intf_l2_mode(L2SW2, "eth0/10", ACCESS);
    node_set_intf_vlan_membership(L2SW2, "eth0/10", 10);
    node_set_intf_l2_mode(L2SW2, "eth0/12", ACCESS);
    node_set_intf_vlan_membership(L2SW2, "eth0/12", 11);

    network_start_packet_receiver_thread(topo);

    return topo;
}




graph_t *
L2_loop_topo(){

#if 0
                                    +---------+                               +----------+
                                    |         |eth0/4                   eth0/4|          |
                                    |  L2SW4  +-------------------------------+  L2SW3   |
                                    |         |TR,V10                   TR,V10|          |                                           
                                    +---+-----+                               +-----+----+                                           
                                        |TR,V10                                     +TR,V10                                          
                                        |eth0/3                                     |eth0/8                                                
                                        |                                           |                                                
                                        |eth0/7,TR,V10                              |eth0/9,TR,V10                                                
                                  +-----+----+                                +-----+---+                                            
                                  |          |                                |         |                                            
   +------+---+                   |          |                                |         |                         +--------+         
   |  H1      |10.1.1.1/24        |   L2SW1  |eth0/5                    eth0/7| L2SW2   |eth0/10           eth0/11|  H6    |         
   |122.1.1.1 +-------------------|          |+-------------------------------|         +-------------+----------+122.1.1.6|         
   +------+---+ eth0/1      eth0/2|          |TR,V10                    TR,V10|         |AC,V10        10.1.1.6/24|        |         
                            AC,V10|          |                                |         |                         +-+------+         
                                  +-----+----+                                +----+----+                                            
#endif

    graph_t *topo = create_new_graph("L2 Loop");
    node_t *H1 = create_new_node(topo, "H1");
    node_set_loopback_address(H1, "122.1.1.1");
    node_t *H6 = create_new_node(topo, "H6");
    create_new_node(H6, "122.1.1.6");
    
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