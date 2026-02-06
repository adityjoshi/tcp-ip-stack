#include "graph.h"
#include "net.h"
#include "layer3/layer3.h"
#include "layer2/layer2.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PATH_LENGTH 20

typedef struct path_node_ {
    node_t *node;
    interface_t *incoming_intf;  // Interface we came from
    unsigned int cost;
    struct path_node_ *prev;
} path_node_t;

// Simple queue for BFS
typedef struct queue_node_ {
    path_node_t *path;
    struct queue_node_ *next;
} queue_node_t;

static queue_node_t *queue_head = NULL;
static queue_node_t *queue_tail = NULL;

static void enqueue(path_node_t *path) {
    queue_node_t *qnode = calloc(1, sizeof(queue_node_t));
    qnode->path = path;
    qnode->next = NULL;
    
    if (!queue_head) {
        queue_head = queue_tail = qnode;
    } else {
        queue_tail->next = qnode;
        queue_tail = qnode;
    }
}

static path_node_t *dequeue() {
    if (!queue_head) return NULL;
    
    queue_node_t *qnode = queue_head;
    queue_head = queue_head->next;
    if (!queue_head) queue_tail = NULL;
    
    path_node_t *path = qnode->path;
    free(qnode);
    return path;
}

static void clear_queue() {
    while (queue_head) {
        dequeue();
    }
}

// Find shortest path from src to dst using BFS
path_node_t *find_path(node_t *src, node_t *dst) {
    if (src == dst) return NULL; // Same node
    
    clear_queue();
    
    // Visited nodes
    bool_t visited[100] = {FALSE}; // Simple array - assumes < 100 nodes
    int node_index = 0;
    node_t *node_map[100];
    
    // Map nodes to indices
    glthread_t *curr;
    node_t *node;
    int idx = 0;
    extern graph_t *topo;
    ITERATE_GLTHREAD_BEGIN(&topo->node_list, curr) {
        node = graph_glue_to_node(curr);
        node_map[idx] = node;
        if (node == src) node_index = idx;
        idx++;
    } ITERATE_GLTHREAD_END(&topo->node_list, curr);
    
    // Start BFS from source
    path_node_t *start = calloc(1, sizeof(path_node_t));
    start->node = src;
    start->incoming_intf = NULL;
    start->cost = 0;
    start->prev = NULL;
    
    enqueue(start);
    visited[node_index] = TRUE;
    
    while (queue_head) {
        path_node_t *current = dequeue();
        
        // Check if we reached destination
        if (current->node == dst) {
            clear_queue(); // Clean up remaining queue
            return current;
        }
        
        // Explore neighbors
        unsigned int i;
        for (i = 0; i < MAX_INTF_PER_NODE; i++) {
            interface_t *intf = current->node->intf[i];
            if (!intf || !intf->link) continue;
            
            node_t *nbr = get_nbr_node(intf);
            if (!nbr) continue;
            
            // Find nbr index
            int nbr_idx = -1;
            int j;
            for (j = 0; j < idx; j++) {
                if (node_map[j] == nbr) {
                    nbr_idx = j;
                    break;
                }
            }
            
            if (nbr_idx == -1 || visited[nbr_idx]) continue;
            
            visited[nbr_idx] = TRUE;
            
            // Create new path node
            path_node_t *next = calloc(1, sizeof(path_node_t));
            next->node = nbr;
            next->incoming_intf = intf; // Interface on current node leading to nbr
            next->cost = current->cost + intf->link->cost;
            next->prev = current;
            
            enqueue(next);
        }
    }
    
    return NULL; // No path found
}

// Print path from destination back to source
void print_path(path_node_t *path_end) {
    if (!path_end) {
        printf("No path found\n");
        return;
    }
    
    // Build path in reverse
    path_node_t *path[MAX_PATH_LENGTH];
    int path_len = 0;
    path_node_t *curr = path_end;
    
    while (curr && path_len < MAX_PATH_LENGTH) {
        path[path_len++] = curr;
        curr = curr->prev;
    }
    
    // Print in forward direction
    printf("Path (cost: %u): ", path_end->cost);
    for (int i = path_len - 1; i >= 0; i--) {
        printf("%s", path[i]->node->node_name);
        if (i > 0 && path[i-1]->incoming_intf) {
            printf(" --[%s]--> ", path[i-1]->incoming_intf->if_name);
        }
    }
    printf("\n");
}

// Free path structure
void free_path(path_node_t *path_end) {
    path_node_t *curr = path_end;
    while (curr) {
        path_node_t *prev = curr->prev;
        free(curr);
        curr = prev;
    }
}

// Find and configure routes for all nodes to reach a destination
// Also blocks redundant ports to prevent loops
void find_and_configure_paths_to_node(node_t *dst_node) {
    extern graph_t *topo;
    glthread_t *curr;
    node_t *node;
    
    // Store best paths and interfaces used
    typedef struct path_info_ {
        node_t *src;
        path_node_t *path;
        interface_t *first_intf;  // First interface in path
    } path_info_t;
    
    path_info_t paths[50];
    int path_count = 0;
    
    printf("\n=== Finding paths to node: %s ===\n", dst_node->node_name);
    
    ITERATE_GLTHREAD_BEGIN(&topo->node_list, curr) {
        node = graph_glue_to_node(curr);
        if (node == dst_node) continue; // Skip destination itself
        
        path_node_t *path = find_path(node, dst_node);
        if (path) {
            printf("\nFrom %s to %s: ", node->node_name, dst_node->node_name);
            print_path(path);
            
            // Store path info
            if (path_count < 50) {
                paths[path_count].src = node;
                paths[path_count].path = path;
                paths[path_count].first_intf = path->prev ? path->prev->incoming_intf : NULL;
                path_count++;
            }
            
            // Configure route if source node has L3 interfaces
            // Find the first L3 hop in the path
            if (path->prev) {
                // Walk back the path to find first L3 node and interface
                path_node_t *curr_path = path;
                interface_t *oif = NULL;
                node_t *next_hop = NULL;
                char *next_hop_ip = NULL;
                
                // Find first hop with L3 interface
                while (curr_path->prev) {
                    if (curr_path->prev->incoming_intf) {
                        interface_t *test_intf = curr_path->prev->incoming_intf;
                        node_t *test_nbr = get_nbr_node(test_intf);
                        
                        // Check if neighbor has L3 interface
                        unsigned int i;
                        for (i = 0; i < MAX_INTF_PER_NODE; i++) {
                            if (test_nbr->intf[i] && test_nbr->intf[i]->link == test_intf->link) {
                                if (IS_INTF_L3_MODE(test_nbr->intf[i])) {
                                    oif = test_intf;
                                    next_hop = test_nbr;
                                    next_hop_ip = (char *)INTERFACE_IP(test_nbr->intf[i]);
                                    break;
                                }
                            }
                        }
                        if (oif) break;
                    }
                    curr_path = curr_path->prev;
                }
                
                // If no L3 hop found, check if we can route through L2
                // For L2 switches, we'll route to the next L3 node
                if (!oif && path->prev && path->prev->incoming_intf) {
                    // Try direct neighbor
                    oif = path->prev->incoming_intf;
                    next_hop = get_nbr_node(oif);
                    
                    unsigned int i;
                    for (i = 0; i < MAX_INTF_PER_NODE; i++) {
                        if (next_hop->intf[i] && next_hop->intf[i]->link == oif->link) {
                            if (IS_INTF_L3_MODE(next_hop->intf[i])) {
                                next_hop_ip = (char *)INTERFACE_IP(next_hop->intf[i]);
                                break;
                            }
                        }
                    }
                }
                
                // Get destination IP
                char dst_ip[16] = "0.0.0.0";
                unsigned int i;
                for (i = 0; i < MAX_INTF_PER_NODE; i++) {
                    if (dst_node->intf[i] && IS_INTF_L3_MODE(dst_node->intf[i])) {
                        strncpy(dst_ip, (char *)INTERFACE_IP(dst_node->intf[i]), 16);
                        dst_ip[15] = '\0';
                        break;
                    }
                }
                
                // Configure route if we have all needed info and source node has L3
                if (oif && next_hop_ip && strncmp(dst_ip, "0.0.0.0", 16) != 0) {
                    // Check if source node has at least one L3 interface
                    bool_t src_has_l3 = FALSE;
                    for (i = 0; i < MAX_INTF_PER_NODE; i++) {
                        if (node->intf[i] && IS_INTF_L3_MODE(node->intf[i])) {
                            src_has_l3 = TRUE;
                            break;
                        }
                    }
                    
                    if (src_has_l3) {
                        // Extract network from IP (assuming /24 for now)
                        char network[16];
                        strncpy(network, dst_ip, 16);
                        char *dot = strrchr(network, '.');
                        if (dot) *dot = '\0';
                        strcat(network, ".0");
                        
                        printf("  Configuring route on %s: %s/24 via %s on %s\n", 
                               node->node_name, network, next_hop_ip, oif->if_name);
                        
                        // Configure the route
                        extern void rt_table_add_route(rt_table_t *rt_table, char *dst, char mask, char *gw, char *oif);
                        rt_table_add_route(Node_RT_TABLE(node), network, (char)24, next_hop_ip, oif->if_name);
                    }
                }
            }
            
            // Don't free path yet - we need it for loop prevention
        } else {
            printf("\nFrom %s to %s: No path found\n", node->node_name, dst_node->node_name);
        }
    } ITERATE_GLTHREAD_END(&topo->node_list, curr);
    
    // Now block redundant ports to prevent loops
    // Strategy: For each switch, identify which interfaces are in the best paths
    // and block all other inter-switch links
    printf("\n=== Blocking redundant ports to prevent loops ===\n");
    
    // Build a map of which interfaces are used in best paths
    typedef struct intf_usage_ {
        interface_t *intf;
        bool_t in_best_path;
    } intf_usage_t;
    
    intf_usage_t intf_usage[200];
    int intf_count = 0;
    
    // Mark all interfaces used in paths
    // For each link in the path, mark both interfaces (sending and receiving)
    int i;
    for (i = 0; i < path_count; i++) {
        path_node_t *p = paths[i].path;
        path_node_t *prev = p->prev;
        while (prev) {
            if (prev->incoming_intf) {
                // Mark the outgoing interface
                int j;
                bool_t found = FALSE;
                for (j = 0; j < intf_count; j++) {
                    if (intf_usage[j].intf == prev->incoming_intf) {
                        intf_usage[j].in_best_path = TRUE;
                        found = TRUE;
                        break;
                    }
                }
                if (!found && intf_count < 200) {
                    intf_usage[intf_count].intf = prev->incoming_intf;
                    intf_usage[intf_count].in_best_path = TRUE;
                    intf_count++;
                }
                
                // Also mark the receiving interface on the neighbor node
                // (the interface on the other side of the link)
                node_t *nbr = get_nbr_node(prev->incoming_intf);
                if (nbr) {
                    unsigned int k;
                    for (k = 0; k < MAX_INTF_PER_NODE; k++) {
                        interface_t *nbr_intf = nbr->intf[k];
                        if (nbr_intf && nbr_intf->link == prev->incoming_intf->link) {
                            // This is the interface on the receiving side
                            found = FALSE;
                            for (j = 0; j < intf_count; j++) {
                                if (intf_usage[j].intf == nbr_intf) {
                                    intf_usage[j].in_best_path = TRUE;
                                    found = TRUE;
                                    break;
                                }
                            }
                            if (!found && intf_count < 200) {
                                intf_usage[intf_count].intf = nbr_intf;
                                intf_usage[intf_count].in_best_path = TRUE;
                                intf_count++;
                            }
                            break;
                        }
                    }
                }
            }
            prev = prev->prev;
        }
    }
    
    // Now block redundant ports on switches
    ITERATE_GLTHREAD_BEGIN(&topo->node_list, curr) {
        node = graph_glue_to_node(curr);
        
        // Check if this is a switch (has L2 interfaces)
        bool_t is_switch = FALSE;
        unsigned int j;
        for (j = 0; j < MAX_INTF_PER_NODE; j++) {
            if (node->intf[j] && (IF_L2_Mode(node->intf[j]) == TRUNK || IF_L2_Mode(node->intf[j]) == ACCESS)) {
                is_switch = TRUE;
                break;
            }
        }
        if (!is_switch) continue;
        
        // Initialize STP if not already initialized
        extern stp_t *get_stp_instance(node_t *node);
        extern void stp_init(node_t *node);
        extern void stp_start(node_t *node);
        stp_t *stp = get_stp_instance(node);
        if (!stp) {
            stp_init(node);
            stp_start(node);
            stp = get_stp_instance(node);
        }
        if (!stp) continue;
        
        // Block all inter-switch trunk ports that are NOT in best paths
        unsigned int k;
        for (k = 0; k < MAX_INTF_PER_NODE; k++) {
            interface_t *intf = node->intf[k];
            if (!intf) break;
            
            // Only block inter-switch links (TRUNK mode)
            if (IF_L2_Mode(intf) == TRUNK && intf->link) {
                node_t *nbr = get_nbr_node(intf);
                // Check if neighbor is a switch (has STP)
                stp_t *nbr_stp = get_stp_instance(nbr);
                
                if (nbr_stp) {
                    // This is an inter-switch link
                    // Check if this interface is in a best path
                    bool_t in_path = FALSE;
                    int m;
                    for (m = 0; m < intf_count; m++) {
                        if (intf_usage[m].intf == intf && intf_usage[m].in_best_path) {
                            in_path = TRUE;
                            break;
                        }
                    }
                    
                    if (!in_path) {
                        // Block this port - it's not in any best path
                        extern void stp_block_port(interface_t *intf);
                        stp_block_port(intf);
                        printf("  Blocked redundant port %s on %s (not in best path)\n", 
                               intf->if_name, node->node_name);
                    } else {
                        // Ensure this port is forwarding
                        extern void stp_unblock_port(interface_t *intf);
                        stp_unblock_port(intf);
                    }
                }
            }
        }
    } ITERATE_GLTHREAD_END(&topo->node_list, curr);
    
    // Free all paths
    for (i = 0; i < path_count; i++) {
        free_path(paths[i].path);
    }
    
    printf("\n=== Path finding and loop prevention complete ===\n\n");
}
