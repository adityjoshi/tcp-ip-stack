# Loop Topology - Routing Loop Demonstration

This topology demonstrates how routing loops can occur when routes are misconfigured.

## Topology Structure

```
     R1 (10.1.1.1) ---- R2 (20.1.1.1)
      |                    |
      |                    |
     R3 (30.1.1.1) --------
```

**Triangle topology with 3 routers:**
- R1: eth0/1 (10.1.1.1/24) connected to R2, eth0/2 (30.1.1.2/24) connected to R3
- R2: eth0/1 (10.1.1.2/24) connected to R1, eth0/2 (20.1.1.1/24) connected to R3
- R3: eth0/1 (20.1.1.2/24) connected to R2, eth0/2 (30.1.1.1/24) connected to R1

## Creating a Routing Loop

To create a loop, configure routes that form a circular path:

### Step 1: Build ARP tables
```
run arp-resolve-all
```

### Step 2: Configure routes to create a loop

**On R1** - Route to R3's network via R2:
```
config node R1 route 20.1.1.0 24 10.1.1.2 eth0/1
config node R1 route 30.1.1.0 24 10.1.1.2 eth0/1
```

**On R2** - Route to R1's network via R3:
```
config node R2 route 10.1.1.0 24 20.1.1.2 eth0/2
config node R2 route 30.1.1.0 24 20.1.1.2 eth0/2
```

**On R3** - Route to R2's network via R1:
```
config node R3 route 10.1.1.0 24 30.1.1.2 eth0/2
config node R3 route 20.1.1.0 24 30.1.1.2 eth0/2
```

### Step 3: Test the loop

Try to ping from R1 to R3's interface:
```
run node R1 ping 20.1.1.2
```

**What happens:**
1. R1 sends packet to 20.1.1.2
2. R1 routes it to R2 (via 10.1.1.2)
3. R2 routes it to R3 (via 20.1.1.2)
4. R3 routes it back to R1 (via 30.1.1.2)
5. R1 routes it to R2 again...
6. **Loop continues until TTL expires (64 hops)**

The packet will loop: **R1 → R2 → R3 → R1 → R2 → ...** until the TTL reaches 0.

## TTL Protection

The IP header includes a TTL (Time To Live) field that prevents infinite loops:
- Initial TTL: 64
- Each router decrements TTL by 1
- When TTL reaches 0, the packet is dropped

You should see the packet being dropped after 64 hops.

## Viewing Routes

Check routing tables:
```
show node R1 rt
show node R2 rt
show node R3 rt
```

## Correct Configuration (No Loop)

To fix the loop, configure direct routes:

**On R1:**
```
config node R1 route 20.1.1.0 24 30.1.1.1 eth0/2  # Direct to R3
```

**On R2:**
```
config node R2 route 10.1.1.0 24 10.1.1.1 eth0/1  # Direct to R1
```

**On R3:**
```
config node R3 route 20.1.1.0 24 20.1.1.1 eth0/1  # Direct to R2
```

This creates a direct path without loops.
