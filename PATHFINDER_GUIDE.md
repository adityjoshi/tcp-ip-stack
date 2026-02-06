# Path Finder Command Guide

## Overview

The `run find-paths <node-name>` command finds the shortest path from all nodes to a destination node and automatically configures routes to prevent loops.

## Usage

```
run find-paths <destination-node-name>
```

## What It Does

1. **Finds Shortest Paths**: Uses BFS (Breadth-First Search) to find the shortest path from every node to the destination
2. **Displays Paths**: Shows the path with interfaces and cost
3. **Configures Routes**: Automatically adds routing table entries to use the best path
4. **Prevents Loops**: By setting explicit routes, prevents infinite loops

## Example

```
run find-paths H2
```

This will:
- Find paths from H1, SW1, SW2, SW3 to H2
- Display each path with interfaces
- Configure routes on each node to reach H2

## Workflow to Avoid Loops

1. **Find paths to destination:**
   ```
   run find-paths H2
   ```

2. **Build ARP tables:**
   ```
   run arp-resolve-all
   ```

3. **Now ping will use the configured paths (no loops!):**
   ```
   run node H1 ping 10.1.1.2
   ```

## How It Works

- Uses BFS algorithm to find shortest paths
- Considers link costs when finding paths
- Automatically extracts network addresses and configures routes
- Routes are added to each node's routing table

## Output Format

```
=== Finding paths to node: H2 ===

From H1 to H2: Path (cost: 2): H1 --[eth0/1]--> SW1 --[eth0/1]--> SW2 --[eth0/1]--> SW3 --[eth0/3]--> H2
  Configuring route: 10.1.1.0/24 via 10.1.1.x on eth0/1

=== Path finding complete ===
```

## Benefits

✅ **Prevents Loops**: Explicit routes prevent infinite loops
✅ **Best Path**: Always uses shortest path
✅ **Automatic**: No manual route configuration needed
✅ **Works with Any Topology**: Finds paths in any connected graph

## Notes

- Paths are found based on link costs
- Routes are configured with /24 mask by default
- Only works for nodes with IP addresses configured
- If a node has no IP, path is shown but route may not be configured
