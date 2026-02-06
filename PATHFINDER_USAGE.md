# Path Finder Usage Guide

## Command

```
run find-paths <destination-node-name>
```

## What It Does

1. **Finds Shortest Paths**: Uses BFS algorithm to find shortest path from every node to the destination
2. **Shows Path Information**: Displays path with interfaces and total cost
3. **Configures Routes**: Automatically adds routing table entries (for L3 nodes)
4. **Prevents Loops**: By using explicit routes, prevents infinite routing loops

## Example Workflow

### Step 1: Find Paths
```
run find-paths H2
```

**Output:**
```
=== Finding paths to node: H2 ===
From H1 to H2: Path (cost: 3): H1 --[eth0/1]--> SW1 --[eth0/2]--> SW3 --[eth0/3]--> H2
From SW3 to H2: Path (cost: 1): SW3 --[eth0/3]--> H2
From SW2 to H2: Path (cost: 2): SW2 --[eth0/2]--> SW3 --[eth0/3]--> H2
From SW1 to H2: Path (cost: 2): SW1 --[eth0/2]--> SW3 --[eth0/3]--> H2
=== Path finding complete ===
```

### Step 2: Build ARP Tables
```
run arp-resolve-all
```

### Step 3: Ping (No Loops!)
```
run node H1 ping 10.1.1.2
```

## How It Prevents Loops

1. **Explicit Routes**: Configures specific routes on each node
2. **Best Path Selection**: Always uses shortest path (lowest cost)
3. **No Ambiguity**: Each node knows exactly which interface to use
4. **Works with STP**: Path finder finds L3 paths, STP prevents L2 loops

## Notes

- **L2 vs L3**: Path finder works best for L3 routing. For L2 (same subnet), STP handles loop prevention
- **Route Configuration**: Routes are only configured on nodes with L3 interfaces
- **Path Cost**: Uses link costs to find shortest path
- **Automatic**: No manual route configuration needed

## When to Use

✅ **Use path finder when:**
- You have routing loops
- You want to see all paths to a destination
- You want automatic route configuration
- You're working with L3 routing

✅ **STP handles:**
- L2 loop prevention
- Broadcast storm prevention
- Layer 2 switching loops

## Troubleshooting

**If ping still loops:**
1. Check that routes were configured: `show node H1 rt`
2. Verify ARP is resolved: `show node H1 arp`
3. Check STP status: `show node SW1 stp` (for L2 loops)

**If no routes configured:**
- Path finder only configures routes on L3 nodes
- L2 switches don't need routes (they use MAC tables)
- Check that destination node has an IP address
