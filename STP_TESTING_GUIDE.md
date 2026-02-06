# STP Testing Guide

## Quick Start

### 1. Build and Run

```bash
cd /Users/adi/Code/tcp/tcp-ip-stack
make clean
make
./test.exe
```

### 2. Basic STP Status Check

Once in the CLI, check STP status on all switches:

```
show node SW1 stp
show node SW2 stp
show node SW3 stp
```

**Expected Output:**
- One switch should be the Root Bridge (lowest bridge ID)
- Other switches should show "Is Root Bridge: No"
- Root ports should be identified
- Port states should be shown (Forwarding/Blocking)

### 3. Test Loop Prevention

#### Step 1: Build ARP Tables
```
run arp-resolve-all
```

#### Step 2: Test Ping
```
run node H1 ping 10.1.1.2
```

#### Step 3: Verify STP Status After Ping
```
show node SW1 stp
show node SW2 stp
show node SW3 stp
```

**What to Look For:**
- At least one port should be in "Blocking" state to break the loop
- Ping should succeed without creating infinite loops
- Only forwarding ports should be used for traffic

## Detailed Testing Scenarios

### Scenario 1: Verify Root Bridge Election

**Commands:**
```
show node SW1 stp
show node SW2 stp
show node SW3 stp
```

**Expected Results:**
- One switch (typically SW3) should have "Is Root Bridge: Yes"
- All switches should agree on the same Root Bridge ID
- Non-root switches should have a Root Port identified

### Scenario 2: Test Loop Prevention

**Commands:**
```
run arp-resolve-all
run node H1 ping 10.1.1.2
show node SW1 stp
show node SW2 stp
show node SW3 stp
```

**Expected Results:**
- Ping should complete successfully
- One port on the triangle loop should be blocked
- No infinite packet loops should occur
- MAC tables should show learned addresses

### Scenario 3: Verify Port States

**Commands:**
```
show node SW1 stp
show node SW2 stp
show node SW3 stp
```

**Check:**
- **Root Bridge (SW3)**: All ports should be "Designated" and "Forwarding"
- **Non-Root Switches**: 
  - One port should be "Root" and "Forwarding" (path to root)
  - Other ports should be "Designated" and "Forwarding" OR "Alternate" and "Blocking"

### Scenario 4: Test MAC Learning with STP

**Commands:**
```
run arp-resolve-all
show node SW1 mac
show node SW2 mac
show node SW3 mac
run node H1 ping 10.1.1.2
show node SW1 mac
show node SW2 mac
show node SW3 mac
```

**Expected Results:**
- MAC addresses should be learned on forwarding ports only
- Blocked ports should not learn MAC addresses
- MAC tables should show correct interface mappings

## Automated Test Script

You can use the provided test script:

```bash
./test_stp.sh
```

Or manually run commands from a file:

```bash
(cat test_stp_commands.txt; echo "quit") | ./test.exe
```

## Understanding the Output

### STP Status Fields

- **Bridge ID**: Unique identifier for this switch (priority + MAC)
- **Root Bridge ID**: The elected root bridge's ID
- **Root Path Cost**: Cost to reach the root bridge
- **Is Root Bridge**: Whether this switch is the root
- **Root Port**: Interface leading to the root bridge (if not root)

### Port Status Fields

- **Interface**: Port name
- **State**: 
  - `Forwarding`: Port actively forwards frames
  - `Blocking`: Port is blocked to prevent loops
  - `Learning`: Port is learning MACs (transitional)
  - `Listening`: Port is in transition (transitional)
  - `Disabled`: Port is disabled

- **Role**:
  - `Root`: Port with best path to root (one per non-root switch)
  - `Designated`: Port that forwards on a segment
  - `Alternate`: Backup port that is blocked
  - `Backup`: Redundant port on same switch (blocked)

- **Cost**: Path cost for this port (default: 19 for 100 Mbps)

## Troubleshooting

### Issue: All ports are Forwarding

**Possible Causes:**
- BPDUs haven't fully converged yet
- Wait a few seconds and check again
- Try sending some traffic to trigger BPDU exchange

**Solution:**
```
run node H1 ping 10.1.1.2
# Wait 2-3 seconds
show node SW1 stp
```

### Issue: No Root Bridge Elected

**Possible Causes:**
- STP not initialized
- BPDUs not being exchanged

**Solution:**
- Verify topology is loaded correctly
- Check that switches have L2 interfaces configured
- Ensure network threads are running

### Issue: Ping Fails or Loops

**Possible Causes:**
- STP not blocking ports correctly
- ARP not resolved

**Solution:**
```
run arp-resolve-all
show node SW1 stp  # Verify blocking ports
run node H1 ping 10.1.1.2
```

## Topology Overview

```
    SW1 ---- SW2
     |        |
     |        |
    SW3 --------
     |
    H1        H2
```

- **SW1, SW2, SW3**: Layer 2 switches with STP enabled
- **H1, H2**: Hosts with IP addresses
- **Triangle Loop**: SW1-SW2-SW3 forms a physical loop
- **STP Role**: Blocks one port to break the loop

## Expected Behavior

1. **Root Election**: SW3 (or switch with lowest bridge ID) becomes root
2. **Root Ports**: SW1 and SW2 select their best port to reach root
3. **Port Blocking**: One port on SW1-SW2 link should be blocked
4. **Traffic Flow**: Ping from H1 to H2 should use only forwarding ports
5. **No Loops**: Packets should not loop indefinitely

## Verification Checklist

- [ ] Root bridge is elected (one switch shows "Is Root Bridge: Yes")
- [ ] All switches agree on root bridge ID
- [ ] Root ports are identified on non-root switches
- [ ] At least one port is in Blocking state (after convergence)
- [ ] Ping succeeds without loops
- [ ] MAC tables show learned addresses
- [ ] Only forwarding ports are used for traffic

## Advanced Testing

### Test Port State Changes

1. Check initial state
2. Send traffic
3. Check state after traffic
4. Verify blocking ports don't forward

### Test Multiple VLANs (if configured)

STP should work per-VLAN if VLANs are configured.

### Test Root Bridge Change

Manually change bridge priorities (if implemented) and verify new root election.
