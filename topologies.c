#include "gluethread/glthread.h"
#include "graph.h"
#include "communication.h"
extern void
network_start_packet_receiver_thread(graph_t *topo);

extern void node_set_intf_l2_mode(node_t *node, char *intf_name, intf_l2_mode_t intf_l2_mode);

graph_t * build_first_topo() {

    // add switch in layer 2 for communication 
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
                                       +----+----+                        +--------+
       +---------+                     |         |                        |        |
       |         |10.1.1.2/24          |   L2Sw  |eth0/2       10.1.1.1/24|  H3    |
       |  H1     +---------------------+         +------------------------+122.1.1.3|
       |122.1.1.1|eth0/5         eth0/4|         |                 eth0/6 |        |
       + --------+                     |         |                        |        |
                                       +----+----+                        +--------+
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
node_set_interface_ip_address(H1, "eth0/5", "10.1.1.2",24);

node_set_loopback_address(H2, "122.1.1.2");
node_set_interface_ip_address(H1, "eth0/5", "10.1.1.4",24);

node_set_loopback_address(H3, "122.1.1.3");
node_set_interface_ip_address(H1, "eth0/5", "10.1.1.1",24);

node_set_loopback_address(H4, "122.1.1.4");
node_set_interface_ip_address(H1, "eth0/5", "10.1.1.3",24);


node_set_intf_l2_mode(L2Sw, "eth0/1", ACCESS);
node_set_intf_l2_mode(L2Sw, "eth0/2", ACCESS);
node_set_intf_l2_mode(L2Sw, "eth0/3", ACCESS);  
node_set_intf_l2_mode(L2Sw, "eth0/4", ACCESS);

network_start_packet_receiver_thread(graph);
return graph; 

}