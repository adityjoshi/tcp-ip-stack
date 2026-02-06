#!/bin/bash

# Test script for STP functionality

echo "=== Testing STP Implementation ==="
echo ""

# Create a test input file
cat > /tmp/stp_test_input.txt << 'EOF'
show node SW1 stp
show node SW2 stp
show node SW3 stp
run arp-resolve-all
run node H1 ping 10.1.1.2
show node SW1 stp
show node SW2 stp
show node SW3 stp
EOF

echo "Running STP test commands..."
timeout 10 ./test.exe < /tmp/stp_test_input.txt 2>&1 | head -100

rm -f /tmp/stp_test_input.txt
