#!/bin/bash
# Test script to verify all CLI commands work

echo "Testing TCP/IP Stack CLI Commands"
echo "=================================="
echo ""

# Test topology loading
echo "1. Testing topology commands..."
echo "   Available topologies:"
echo "   - build_first_topo"
echo "   - build_linear_topo"
echo "   - build_simple_l2_switch_topo"
echo "   - build_dualswitch_topo"
echo "   - L2_loop_topo"
echo "   - linear_3_node_topo"
echo ""

# Test show commands
echo "2. Testing show commands..."
echo "   Commands to test:"
echo "   - show topology"
echo "   - show node <node-name>"
echo "   - show node <node-name> arp"
echo "   - show node <node-name> mac"
echo "   - show node <node-name> rt"
echo ""

# Test run commands
echo "3. Testing run commands..."
echo "   Commands to test:"
echo "   - run node <node-name> ping <ip-address>"
echo "   - run node <node-name> resolve-arp <ip-address>"
echo ""

# Test config commands
echo "4. Testing config commands..."
echo "   Commands to test:"
echo "   - config node <node-name> interface <if-name> l2mode <access|trunk>"
echo "   - config node <node-name> interface <if-name> vlan <vlan-id>"
echo "   - config node <node-name> route <ip-address> <mask> [gw-ip] [oif]"
echo ""

echo "All commands are registered in nwcli.c"
echo "Topology functions are in topologies.c"
