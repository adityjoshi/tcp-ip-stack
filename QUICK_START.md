# Quick Start Guide

## Running the Program

1. **Start the program:**
   ```bash
   ./test.exe
   ```

2. **Wait 1-2 seconds** for initialization. You'll see:
   - Debug messages about topology setup
   - BPDU processing messages
   - Then the CLI prompt: `tcp-ip-project> $`

3. **The CLI is ready when you see:**
   ```
   tcp-ip-project> $ 
   ```

## First Commands to Try

Once you see the prompt, try:

```
show help                    # See available commands
show node SW1 stp           # Check STP status
show node SW2 stp
show node SW3 stp
run arp-resolve-all          # Build ARP tables
run node H1 ping 10.1.1.2    # Test ping
show node SW1 mac            # See MAC table
```

## Exiting

To exit, press `Ctrl+C` or type:
```
cd                          # Go to top of command tree
```

## If CLI Doesn't Appear

1. **Wait longer** - initialization takes 1-2 seconds
2. **Type anyway** - the prompt might be there but not visible
3. **Check terminal** - make sure your terminal supports interactive input
4. **Try this:**
   ```bash
   ./test.exe
   # Wait 2 seconds, then type:
   show help
   ```

## Testing STP Loop Prevention

```
show node SW1 stp           # Check initial STP state
run arp-resolve-all         # Build ARP tables
run node H1 ping 10.1.1.2   # Test ping - should succeed without loops
show node SW1 stp           # Check STP after traffic
show node SW1 mac           # See which ports learned MACs
```

## Expected Behavior

- ✅ CLI prompt appears after 1-2 seconds
- ✅ Commands execute successfully
- ✅ Ping succeeds without infinite loops
- ✅ STP prevents loops by blocking redundant ports
