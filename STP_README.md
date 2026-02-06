# Spanning Tree Protocol (STP) Implementation

## Overview

The Spanning Tree Protocol (STP) prevents loops in Layer 2 switched networks by creating a loop-free logical topology. When multiple paths exist between switches, STP blocks redundant ports to ensure only one active path exists.

## Features

- **Root Bridge Election**: Automatically elects the root bridge based on lowest Bridge ID
- **Port States**: Ports can be in Forwarding, Blocking, Listening, Learning, or Disabled states
- **Port Roles**: Root Port, Designated Port, Alternate Port, Backup Port
- **BPDU Processing**: Handles Bridge Protocol Data Units for topology calculation
- **Automatic Loop Prevention**: Blocks ports to prevent broadcast storms

## STP Topology

The `build_stp_loop_topo()` creates a triangle topology with 3 switches:

```
    SW1 ---- SW2
     |        |
     |        |
    SW3 --------
```

**Without STP**: This topology would create a loop, causing broadcast storms.
**With STP**: One port will be blocked, breaking the loop.

## Usage

### 1. View STP Status

Check STP status on any switch:

```
show node SW1 stp
show node SW2 stp
show node SW3 stp
```

This will show:
- Bridge ID
- Root Bridge ID
- Root Path Cost
- Whether this switch is the root bridge
- Root Port (if not root bridge)
- Port status (Interface, State, Role, Cost)

### 2. Test Loop Prevention

1. **Build ARP tables** (for L3 connectivity):
   ```
   run arp-resolve-all
   ```

2. **Ping from H1 to H2**:
   ```
   run node H1 ping 10.1.1.2
   ```

3. **Check STP status** to see which port is blocked:
   ```
   show node SW1 stp
   show node SW2 stp
   show node SW3 stp
   ```

4. **Verify no loops**: The ping should succeed without creating infinite loops. One port should be in "Blocking" state.

### 3. View MAC Tables

Check MAC learning on switches:

```
show node SW1 mac
show node SW2 mac
show node SW3 mac
```

## STP Port States

- **Forwarding**: Port actively forwards frames
- **Blocking**: Port is blocked to prevent loops (no forwarding)
- **Learning**: Port is learning MAC addresses but not forwarding
- **Listening**: Port is in transition state
- **Disabled**: Port is administratively disabled

## STP Port Roles

- **Root Port**: Port with best path to root bridge (one per non-root bridge)
- **Designated Port**: Port that forwards frames on a segment (one per segment)
- **Alternate Port**: Backup port that is blocked
- **Backup Port**: Redundant port on same switch (blocked)

## How STP Works

1. **Root Bridge Election**: All switches exchange BPDUs. The switch with the lowest Bridge ID becomes the root bridge.

2. **Root Port Selection**: Each non-root switch selects one root port (the port with the best path to the root).

3. **Designated Port Selection**: On each segment, one port becomes designated (the port closest to the root).

4. **Blocking**: All other ports are blocked to prevent loops.

5. **Forwarding**: Only root ports and designated ports forward frames.

## Example Output

```
=== STP Status for Node: SW1 ===
Bridge ID: 0x800000000001
Root Bridge ID: 0x800000000001
Root Path Cost: 0
Is Root Bridge: Yes
Root Port: (none)

Port Status:
Interface       State        Role         Cost
------------------------------------------------------------
eth0/1          Forwarding   Designated   19
eth0/2          Forwarding   Designated   19
eth0/3          Forwarding   Designated   19
```

## Integration with L2 Switching

STP is automatically integrated with Layer 2 switching:
- The `l2_switch_flood_pkt_out()` function checks STP port state before forwarding
- Only ports in "Forwarding" state will forward frames
- Blocked ports prevent frame forwarding, breaking loops

## Notes

- STP is automatically initialized when `stp_start()` is called on a node
- STP only applies to L2 interfaces (ACCESS or TRUNK mode)
- The topology uses a simplified STP implementation for demonstration
- In production, STP uses timers and state machines for convergence
