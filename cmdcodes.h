#ifndef __CMDCODES__
#define __CMDCODES__

#define CMDCODE_SHOW_NW_TOPOLOGY    1   /*show topology*/
#define CMDCODE_PING                2   /*run <node-name> ping <dst ip address>*/
#define CMDCODE_SHOW_NODE_ARP_TABLE 3   /*show node <node-name> arp*/
#define CMDCODE_RUN_ARP             4   /*run node <node-name> resolve-arp <ip-address>*/
#define CMDCODE_INTF_CONFIG_L2_MODE 5   /*config node <node-name> interface <intf-name> l2mode <access|trunk>*/
#define CMDCODE_INTF_CONFIG_IP_ADDR 6   /*config node <node-name> interface <intf-name> ip-address <ip-address> <mask>*/
#define CMDCODE_INTF_CONFIG_VLAN    7   /*config node <node-name> interface <intf-name> vlan <vlan-id>*/
#define CMDCODE_SHOW_NODE_MAC_TABLE 8   /*show node <node-name> mac*/
#define CMDCODE_SHOW_NODE_RT_TABLE  9   /*show node <node-name> rt*/
#define CMDCODE_SHOW_NODE_STP       13  /*show node <node-name> stp*/
#define CMDCODE_CONF_NODE_L3ROUTE   10  /*config node <node-name> route <ip-address> <mask> [<gw-ip> <oif>]*/
#define CMDCODE_RUN_PING            11  /*run node <node-name> ping <ip-address>*/
#define CMDCODE_RUN_ARP_RESOLVE_ALL 12  /*run arp-resolve-all*/
#define CMDCODE_FIND_PATHS          14  /*run find-paths <node-name>*/
#define CMDCODE_MAN                 15  /*man [command-name]*/
#define CMDCODE_ROUTE_CONFIGURE_ALL 16  /*run route-configure-all*/
#endif /* __CMDCODES__ */
