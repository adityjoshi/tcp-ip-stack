# TCP/IP Stack 

This repository contains a layered implementation of core components of the TCP/IP stack, starting from the data link layer up to the network layer. It includes a virtual emulation of network topologies with switches and routers, providing a view into how network protocols operate and interact.

---

## Overview

The project starts by building a multi-node topology emulation environment consisting of virtual routers and switches. This forms the basis for simulating realistic packet forwarding scenarios.

The data link layer is implemented with support for Ethernet framing and ARP. MAC address learning and forwarding logic is written from scratch to simulate L2 switch behavior. To support segmentation and isolation, VLAN-aware switching is added using 802.1Q tagging. The network layer builds on this with manual construction and parsing of IPv4 packets, along with IP routing logic across subnets and interfaces.

---

## Components

- **Ethernet framing and parsing**: Manual construction of Ethernet frames, type identification, and payload decoding.
- **Address Resolution Protocol (ARP)**: Implements ARP request/reply behavior and maintains ARP tables.
- **MAC-based L2 switching**: Learns MAC addresses per interface, supports unicast and broadcast forwarding.
- **VLAN-aware switching**: Supports 802.1Q VLAN tagging, maintains separate MAC tables per VLAN, and enforces VLAN-based forwarding boundaries.
- **IPv4 routing**: Parses and constructs IP headers, performs longest-prefix match for routing, and maintains routing tables with interfaces.

---

## Implementation Details

- All protocol layers interact through clearly defined interfaces.
- Raw packet manipulation is performed at the byte level for both transmission and parsing.
- Each node in the emulated network can act as a host, switch, or router depending on configuration.
- MAC and IP forwarding logic is implemented independently to mirror real-world decoupling of L2 and L3.

---
