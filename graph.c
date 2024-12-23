#include "gluethread/gltthread.h"
#include "graph.h"


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

graph_t *topo = create_new_graph("Generic Graph");
node_t *R0_re = create_new_node(topo,"R0_re");
node_t *R1_re = create_new_node(topo,"R1_re");
node_t *R2_re = create_new_node(topo,"R2_re");
}
