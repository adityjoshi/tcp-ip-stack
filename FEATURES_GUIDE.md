# TCP/IP Stack - New Features Guide

## Overview

This guide explains the new features added to the TCP/IP stack and which topologies to use for testing each feature.

---

## 🚀 New Commands

### 1. `run find-paths <node-name>`
**Purpose**: Find shortest paths to a destination and prevent Layer 2 loops

**What it does**:
- Uses BFS to find shortest paths from all nodes to destination
- Configures static routes on L3 nodes along best paths
- Blocks redundant inter-switch ports using STP to prevent loops
- Automatically initializes STP on switches if needed

**Topology to use**: `build_loop_demo_topo()` (default in testapp.c)
```bash
run find-paths H2
show node SW1 stp    # See blocked ports
run node H1 ping 10.1.1.2
```

**Why**: This topology has L2 loops that need blocking. The pathfinder breaks the loop.

---

### 2. `run route-configure-all`
**Purpose**: Automatically configure routing tables for all nodes

**What it does**:
- Adds direct routes to all directly connected neighbor subnets
- Only processes nodes with L3 interfaces
- Skips routes that already exist

**Topology to use**: `build_route_test_topo()`
```bash
run route-configure-all
show node R1 rt
show node R3 rt
```

**Why**: This topology has multiple routers on different subnets, perfect for testing route configuration.

---

### 3. `run arp-resolve-all`
**Purpose**: Build ARP tables for all nodes automatically

**What it does**:
- Resolves ARP for all directly connected neighbors
- Resolves ARP for all gateway IPs in routing tables
- Populates ARP tables before testing connectivity

**Topology to use**: Any topology with L3 nodes
```bash
run arp-resolve-all
run node H1 ping 10.1.1.2
```

**Why**: Required before pinging - ensures MAC addresses are known.

---

### 4. `man [command-name]`
**Purpose**: Display manual pages for CLI commands

**What it does**:
- Shows detailed documentation for any command
- Lists all available commands when run without arguments
- Includes syntax, description, examples, and notes

**Topology to use**: Any topology
```bash
man                    # List all commands
man ping              # Get help on ping
man find-paths        # Get help on pathfinder
man route-configure-all
```

---

## 🔧 New Features

### Spanning Tree Protocol (STP)
**Purpose**: Prevent Layer 2 loops in switched networks

**Commands**:
- `show node <node-name> stp` - Display STP status
- Automatically used by `run find-paths` to block ports

**Topology to use**: `build_loop_demo_topo()` or `build_stp_loop_topo()`
```bash
run find-paths H2      # Automatically initializes and uses STP
show node SW1 stp      # See port states (Forwarding/Blocking)
```

---

## 📋 Topology Reference

### `build_loop_demo_topo()`
**Use for**: Testing pathfinder and loop prevention
- Triangle loop: SW1-SW2-SW3-SW1
- Hosts H1 and H2 connected to switches
- **No automatic STP blocking** - demonstrates the problem
- **Solution**: Run `run find-paths H2` to break the loop

**Test commands**:
```bash
run find-paths H2
show node SW1 stp
run arp-resolve-all
run node H1 ping 10.1.1.2
```

---

### `build_route_test_topo()`
**Use for**: Testing route configuration
- Multiple routers (R1, R2, R3, R4) on different subnets
- Hosts H1 and H2 connected to routers
- Multiple interfaces per router

**Test commands**:
```bash
run route-configure-all
show node R1 rt
show node R3 rt
run arp-resolve-all
run node H1 ping 40.1.1.10
```

---

### `build_stp_loop_topo()`
**Use for**: Testing STP convergence
- Similar to loop_demo but with automatic STP initialization
- STP starts automatically on all switches

**Test commands**:
```bash
show node SW1 stp
show node SW2 stp
show node SW3 stp
run arp-resolve-all
run node H1 ping 10.1.1.2
```

---

## 🎯 Quick Testing Workflow

### For Loop Prevention Testing:
```bash
# 1. Start with loop_demo topology (default)
./test.exe

# 2. Show the problem (infinite loop)
run arp-resolve-all
run node H1 ping 10.1.1.2    # Will loop if no blocking

# 3. Apply solution
run find-paths H2            # Blocks redundant ports

# 4. Verify
show node SW1 stp            # See blocked ports
run node H1 ping 10.1.1.2   # Should work now
```

### For Route Configuration Testing:
```bash
# 1. Switch to route_test topology in testapp.c
#    Change: topo = build_route_test_topo();

# 2. Configure routes
run route-configure-all

# 3. Verify routing tables
show node R1 rt
show node R3 rt

# 4. Test connectivity
run arp-resolve-all
run node H1 ping 40.1.1.10
```

---

## 📝 Summary Table

| Feature | Command | Topology | Purpose |
|---------|---------|----------|---------|
| Pathfinder | `run find-paths <node>` | `build_loop_demo_topo()` | Prevent L2 loops |
| Route Config | `run route-configure-all` | `build_route_test_topo()` | Auto-configure routes |
| ARP Build | `run arp-resolve-all` | Any | Build ARP tables |
| STP Status | `show node <name> stp` | `build_loop_demo_topo()` | View STP state |
| Manual Pages | `man [cmd]` | Any | Get command help |

---

## 💡 Tips

1. **Always run `arp-resolve-all` before pinging** - Required for MAC address resolution
2. **Use `find-paths` for loop prevention** - Automatically blocks redundant ports
3. **Use `route-configure-all` after topology changes** - Rebuilds routing tables
4. **Check STP status** - Use `show node <name> stp` to see blocked/forwarding ports
5. **Use `man` for help** - Get detailed documentation on any command

---

## 🔄 Changing Topologies

To switch topologies, edit `testapp.c`:

```c
// For loop testing
topo = build_loop_demo_topo();

// For route testing  
topo = build_route_test_topo();

// For STP testing
topo = build_stp_loop_topo();
```

Then recompile: `make`
