#include "graph.h"
#include "CommandParser/libcli.h"
#include "communication.h"
#include "unistd.h"
#include <stdio.h>



extern graph_t *build_first_topo();
extern graph_t *build_linear_topo();
extern graph_t *build_simple_l2_switch_topo();
extern graph_t *build_dualswitch_topo();
extern graph_t *L2_loop_topo();
extern graph_t *linear_3_node_topo();
extern void nw_init_cli();



graph_t *topo = NULL ;

int main(int argc, char **argv) {
  
nw_init_cli();
topo = linear_3_node_topo();


start_shell();
return 0; 
}
