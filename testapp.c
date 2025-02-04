#include "graph.h"
#include "CommandParser/libcli.h"
extern graph_t *build_first_topo();
extern void nw_init_cli();
graph_t *topo = NULL ;

int main(int argc, char **argv) {

nw_init_cli();
topo = build_first_topo();
// node_t *R0_re = create_new_node(topo, "R0");
// node_t *R1_re = create_new_node(topo, "R1");
// node_t *R2_re = create_new_node(topo, "R2");

//dump_graph(topo);

start_shell();
return 0; 
}
