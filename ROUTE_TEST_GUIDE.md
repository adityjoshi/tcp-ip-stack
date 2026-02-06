# Route Configuration Test Topology Guide

## Overview

The `build_route_test_topo()` topology is designed to test the `run route-configure-all` command. It features multiple routers on different subnets to demonstrate automatic route configuration.

## Topology Structure

```
     R1 (10.1.1.1/24)          R2 (20.1.1.1/24)
        |                          |
        |eth0/1                eth0/1|
        |                          |
        +----------+----------+-----+
                   |          |
                eth0/2     eth0/2
                   |          |
                +--+--+      +--+--+
                | R3  |------| R4  |
                +--+--+      +--+--+
                   |          |
                eth0/3     eth0/3
                   |          |
                +--+--+      +--+--+
                | H1  |      | H2  |
                +-----+      +-----+
              (30.1.1.10)   (40.1.1.10)
```

## Subnet Configuration

- **R1-R3 Link**: 10.1.1.0/24
  - R1: 10.1.1.1/24
  - R3: 10.1.1.2/24

- **R2-R4 Link**: 20.1.1.0/24
  - R2: 20.1.1.1/24
  - R4: 20.1.1.2/24

- **R3-R4 Link**: 30.1.1.0/24
  - R3: 30.1.1.1/24
  - R4: 30.1.1.3/24

- **R3-H1 Link**: 30.1.1.0/24
  - R3: 30.1.1.2/24
  - H1: 30.1.1.10/24

- **R4-H2 Link**: 40.1.1.0/24
  - R4: 40.1.1.1/24
  - H2: 40.1.1.10/24

## Testing route-configure-all

### Step 1: View Topology

```bash
./test.exe
show topology
```

### Step 2: Check Initial Routing Tables

```bash
show node R1 rt
show node R2 rt
show node R3 rt
show node R4 rt
```

**Note**: Direct routes are automatically added when IP addresses are configured, so you'll see routes to directly connected subnets already present.

### Step 3: Run route-configure-all

```bash
run route-configure-all
```

**Expected Output**:
```
=== Configuring routing tables for all nodes ===

Processing node: R1
  Route to 10.1.1.0/24 already exists, skipping

Processing node: R2
  Route to 20.1.1.0/24 already exists, skipping

Processing node: R3
  Route to 10.1.1.0/24 already exists, skipping
  Route to 30.1.1.0/24 already exists, skipping

Processing node: R4
  Route to 20.1.1.0/24 already exists, skipping
  Route to 30.1.1.0/24 already exists, skipping
  Route to 40.1.1.0/24 already exists, skipping

Processing node: H1
  Route to 30.1.1.0/24 already exists, skipping

Processing node: H2
  Route to 40.1.1.0/24 already exists, skipping

=== Routing table configuration complete ===
```

### Step 4: Verify Routing Tables

```bash
show node R1 rt
show node R3 rt
show node R4 rt
```

## Understanding the Output

The `route-configure-all` command:
1. **Iterates through all nodes** with L3 interfaces
2. **Finds directly connected neighbors** on each interface
3. **Calculates neighbor subnets** from their IP and mask
4. **Adds direct routes** to those subnets
5. **Skips routes** that already exist

Since direct routes are automatically added when IPs are configured (via `node_set_interface_ip_address`), most routes will already exist. However, the command is useful for:
- Adding routes that might have been missed
- Rebuilding routing tables after topology changes
- Ensuring all neighbor routes are present

## Testing Inter-Subnet Routing

To test routing between different subnets, you can:

1. **Build ARP tables**:
   ```bash
   run arp-resolve-all
   ```

2. **Add routes for inter-subnet communication** (if needed):
   ```bash
   # Example: Add route on R1 to reach 20.1.1.0/24 via R3
   config node R1 route 20.1.1.0 24 10.1.1.2 eth0/1
   ```

3. **Or use pathfinder** to automatically configure routes:
   ```bash
   run find-paths H2
   ```

4. **Test connectivity**:
   ```bash
   run node H1 ping 40.1.1.10
   ```

## Key Features

- **Multiple routers** on different subnets
- **Hosts connected** to routers
- **Multiple interfaces per router** to test complex routing
- **Different subnet ranges** to clearly see route configuration

## Notes

- Direct routes are automatically added when IPs are configured
- The `route-configure-all` command ensures all neighbor routes exist
- For inter-subnet routing, use `run find-paths <node>` or manually configure routes
- Always run `run arp-resolve-all` before testing connectivity with ping
