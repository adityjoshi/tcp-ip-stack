# Generic Graph Construction 

### What is a Graph?

A graph is essentially a collection of nodes.

<p align="center">
  <img width="513" alt="Graph Example" src="https://github.com/user-attachments/assets/d6b33bb0-fe63-48eb-a1ed-9ae6616fb61a" />
</p>

### Exploring the Graph

In the example above, we have 3 nodes (think of nodes as routers), and each node has two interfaces. But what exactly are interfaces?

### An interface includes:
- **Name** (e.g., `eth0/0`)
- **Node** (the connected node)
- **Wire** (a wire or link connecting two nodes)

### Properties of a Node

- A node has a **name**.
- A node has an **empty slot** where a link or wire can be connected.

Understanding these properties of interfaces and nodes will guide us in building our struct logic.
