#include "graph.h"
#include "CommandParser/libcli.h"
#include "communication.h"
#include "unistd.h"
extern graph_t *build_first_topo();
extern graph_t *build_linear_topo();
extern graph_t *build_simple_l2_switch_topo();
extern graph_t *build_dualswitch_topo();
extern void nw_init_cli();
graph_t *topo = NULL ;

int main(int argc, char **argv) {
  
nw_init_cli();
topo = build_dualswitch_topo();


start_shell();
return 0; 
}
