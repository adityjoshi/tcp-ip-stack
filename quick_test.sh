#!/bin/bash

echo "=== Quick STP Test ==="
echo ""
echo "1. Checking STP status on all switches..."
echo ""
echo "show node SW1 stp" > /tmp/stp_test.txt
echo "show node SW2 stp" >> /tmp/stp_test.txt
echo "show node SW3 stp" >> /tmp/stp_test.txt
echo "run arp-resolve-all" >> /tmp/stp_test.txt
echo "run node H1 ping 10.1.1.2" >> /tmp/stp_test.txt
echo "show node SW1 stp" >> /tmp/stp_test.txt
echo "show node SW2 stp" >> /tmp/stp_test.txt
echo "show node SW3 stp" >> /tmp/stp_test.txt
echo "quit" >> /tmp/stp_test.txt

echo "Running test commands..."
echo ""
./test.exe < /tmp/stp_test.txt 2>&1 | grep -E "STP Status|Bridge ID|Root Bridge|Is Root|Interface|State|Role|Blocking|Forwarding|ping success|IP Address|Building ARP" | head -50

rm -f /tmp/stp_test.txt
echo ""
echo "=== Test Complete ==="
