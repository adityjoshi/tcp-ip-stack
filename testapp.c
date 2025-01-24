#include "graph.h"

extern graph_t *build_first_topo();

int main(int argc, char **argv) {
graph_t *topo = build_first_topo();
node_t *R0_re = create_new_node(topo, "R0");
node_t *R1_re = create_new_node(topo, "R1");
node_t *R2_re = create_new_node(topo, "R2");

//dump_graph(topo);
dump_nw_graph(topo);

return 0; 
}
