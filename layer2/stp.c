#include "stp.h"
#include "net.h"
#include "graph.h"
#include "communication.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "layer2/layer2.h"

#define STP_MAC_ADDRESS {0x01, 0x80, 0xC2, 0x00, 0x00, 0x00}
#define STP_ETHER_TYPE 0x0000

static unsigned char stp_multicast_mac[] = STP_MAC_ADDRESS;

// Forward declarations
static unsigned long long stp_ntohll(unsigned long long value);
static unsigned long long stp_htonll(unsigned long long value);

static unsigned long long create_bridge_id(unsigned short priority, char *mac) {
    unsigned long long bridge_id = 0;
    bridge_id = ((unsigned long long)priority << 48);
    bridge_id |= ((unsigned long long)(unsigned char)mac[0] << 40);
    bridge_id |= ((unsigned long long)(unsigned char)mac[1] << 32);
    bridge_id |= ((unsigned long long)(unsigned char)mac[2] << 24);
    bridge_id |= ((unsigned long long)(unsigned char)mac[3] << 16);
    bridge_id |= ((unsigned long long)(unsigned char)mac[4] << 8);
    bridge_id |= ((unsigned long long)(unsigned char)mac[5]);
    return bridge_id;
}

static unsigned short get_bridge_priority(unsigned long long bridge_id) {
    return (unsigned short)(bridge_id >> 48);
}

static void stp_init_port(stp_port_t *stp_port, interface_t *intf) {
    memset(stp_port, 0, sizeof(stp_port_t));
    stp_port->intf = intf;
    stp_port->state = STP_PORT_STATE_BLOCKING;
    stp_port->role = STP_PORT_ROLE_ALTERNATE;
    // Use interface slot index for port ID
    unsigned int port_num = 1;
    node_t *node = intf->att_node;
    unsigned int i;
    for (i = 0; i < MAX_INTF_PER_NODE; i++) {
        if (node->intf[i] == intf) {
            port_num = i + 1;
            break;
        }
    }
    stp_port->port_id = STP_DEFAULT_PORT_PRIORITY << 8 | port_num;
    stp_port->path_cost = STP_DEFAULT_COST;
    stp_port->is_edge_port = FALSE;
    init_glthread(&stp_port->stp_port_glue);
}

void stp_init(node_t *node) {
    stp_t *stp = calloc(1, sizeof(stp_t));
    stp->node = node;
    
    // Create bridge ID from priority and first interface MAC address
    unsigned short priority = STP_DEFAULT_BRIDGE_PRIORITY;
    char *mac = NULL;
    // Use first L2 interface MAC as bridge MAC
    unsigned int i;
    for (i = 0; i < MAX_INTF_PER_NODE; i++) {
        if (node->intf[i] && (IF_L2_Mode(node->intf[i]) == ACCESS || IF_L2_Mode(node->intf[i]) == TRUNK)) {
            mac = (char *)INTERFACE_MAC(node->intf[i]);
            break;
        }
    }
    if (!mac) {
        // No L2 interface found, use first interface
        if (node->intf[0]) {
            mac = (char *)INTERFACE_MAC(node->intf[0]);
        } else {
            // Fallback - use node name hash
            unsigned char fallback_mac[6] = {0};
            unsigned long long hash = 0;
            for (i = 0; i < strlen(node->node_name) && i < 6; i++) {
                fallback_mac[i] = node->node_name[i];
            }
            mac = (char *)fallback_mac;
        }
    }
    stp->bridge_id = create_bridge_id(priority, mac);
    stp->root_bridge_id = stp->bridge_id;
    stp->root_path_cost = 0;
    stp->root_port = NULL;
    stp->is_root_bridge = FALSE;
    init_glthread(&stp->stp_port_list);
    
    // Initialize STP ports for all L2 interfaces
    unsigned int k;
    for (k = 0; k < MAX_INTF_PER_NODE; k++) {
        interface_t *intf = node->intf[k];
        if (!intf) break;
        
        // Only create STP port for L2 interfaces
        if (IF_L2_Mode(intf) == ACCESS || IF_L2_Mode(intf) == TRUNK) {
            stp_port_t *stp_port = calloc(1, sizeof(stp_port_t));
            stp_init_port(stp_port, intf);
            stp->stp_ports[k] = stp_port;
            glthread_add_next(&stp->stp_port_list, &stp_port->stp_port_glue);
        }
    }
    
    // Store STP instance in node (we'll need to add this to net.h)
    // For now, use a simple approach - store in node's extension
    node->stp = stp;
}

stp_t *get_stp_instance(node_t *node) {
    return node->stp;
}

stp_port_t *get_stp_port(interface_t *intf) {
    if (!intf || !intf->att_node) return NULL;
    stp_t *stp = get_stp_instance(intf->att_node);
    if (!stp) return NULL;
    
    unsigned int i;
    for (i = 0; i < MAX_INTF_PER_NODE; i++) {
        if (stp->stp_ports[i] && stp->stp_ports[i]->intf == intf) {
            return stp->stp_ports[i];
        }
    }
    return NULL;
}

bool_t stp_is_port_forwarding(interface_t *intf) {
    stp_port_t *stp_port = get_stp_port(intf);
    if (!stp_port) return TRUE; // If no STP, allow forwarding
    
    return (stp_port->state == STP_PORT_STATE_FORWARDING);
}

static int compare_bridge_id(unsigned long long id1, unsigned long long id2) {
    if (id1 < id2) return -1;
    if (id1 > id2) return 1;
    return 0;
}

void stp_process_bpdu(interface_t *intf, void *bpdu_ptr) {
    stp_bpdu_t *bpdu = (stp_bpdu_t *)bpdu_ptr;
    stp_t *stp = get_stp_instance(intf->att_node);
    if (!stp) return;
    
    stp_port_t *stp_port = get_stp_port(intf);
    if (!stp_port) return;
    
    // Convert network byte order (using our custom functions to avoid conflict with system macros)
    bpdu->root_bridge_id = stp_ntohll(bpdu->root_bridge_id);
    bpdu->bridge_id = stp_ntohll(bpdu->bridge_id);
    bpdu->root_path_cost = ntohl(bpdu->root_path_cost);
    bpdu->port_id = ntohs(bpdu->port_id);
    
    // Check if received BPDU has better root
    int cmp = compare_bridge_id(bpdu->root_bridge_id, stp->root_bridge_id);
    
    // If we receive a BPDU with a better (lower) root bridge ID, update our root
    if (cmp < 0) {
        // Better root found
        stp->root_bridge_id = bpdu->root_bridge_id;
        stp->root_path_cost = bpdu->root_path_cost + stp_port->path_cost;
        stp->root_port = intf;
        stp->is_root_bridge = FALSE;
        
        // Update port role and state
        stp_port->role = STP_PORT_ROLE_ROOT;
        stp_port->state = STP_PORT_STATE_FORWARDING;
        
        // Update other ports
        unsigned int i;
        for (i = 0; i < MAX_INTF_PER_NODE; i++) {
            if (stp->stp_ports[i] && stp->stp_ports[i] != stp_port) {
                if (stp->stp_ports[i]->intf->link) {
                    stp->stp_ports[i]->role = STP_PORT_ROLE_DESIGNATED;
                    stp->stp_ports[i]->state = STP_PORT_STATE_FORWARDING;
                } else {
                    stp->stp_ports[i]->state = STP_PORT_STATE_BLOCKING;
                }
            }
        }
    } else if (cmp == 0) {
        // Same root bridge - check if this is a better path to root
        unsigned int new_path_cost = bpdu->root_path_cost + stp_port->path_cost;
        
        if (new_path_cost < stp->root_path_cost) {
            // Better path to root found
            stp->root_path_cost = new_path_cost;
            if (stp->root_port != intf) {
                // New root port
                if (stp->root_port) {
                    stp_port_t *old_root_port = get_stp_port(stp->root_port);
                    if (old_root_port) {
                        old_root_port->role = STP_PORT_ROLE_ALTERNATE;
                        old_root_port->state = STP_PORT_STATE_BLOCKING;
                    }
                }
                stp->root_port = intf;
                stp_port->role = STP_PORT_ROLE_ROOT;
                stp_port->state = STP_PORT_STATE_FORWARDING;
                
                // Re-evaluate other ports and send BPDUs
                unsigned int i;
                for (i = 0; i < MAX_INTF_PER_NODE; i++) {
                    if (stp->stp_ports[i] && stp->stp_ports[i] != stp_port && 
                        stp->stp_ports[i]->intf->link) {
                        stp_send_bpdu(stp->stp_ports[i]->intf);
                    }
                }
            }
        } else if (compare_bridge_id(bpdu->bridge_id, stp->bridge_id) == 0) {
            // BPDU from ourselves - ignore
            return;
        } else {
            // Same root, same or worse path - determine designated port
            // This is for port role determination on a segment
            unsigned int our_path_cost = stp->root_path_cost;
            unsigned int their_path_cost = bpdu->root_path_cost + stp_port->path_cost;
            
            if (their_path_cost < our_path_cost) {
                // Their path is better - we should block this port
                stp_port->role = STP_PORT_ROLE_ALTERNATE;
                stp_port->state = STP_PORT_STATE_BLOCKING;
            } else if (their_path_cost > our_path_cost) {
                // Our path is better - we should be designated
                stp_port->role = STP_PORT_ROLE_DESIGNATED;
                stp_port->state = STP_PORT_STATE_FORWARDING;
                stp_send_bpdu(intf); // Send our BPDU to claim designated role
            } else {
                // Same path cost - compare bridge IDs
                int bridge_cmp = compare_bridge_id(bpdu->bridge_id, stp->bridge_id);
                if (bridge_cmp < 0) {
                    // Their bridge ID is lower - they should be designated, we block
                    stp_port->role = STP_PORT_ROLE_ALTERNATE;
                    stp_port->state = STP_PORT_STATE_BLOCKING;
                } else {
                    // Our bridge ID is lower - we should be designated
                    stp_port->role = STP_PORT_ROLE_DESIGNATED;
                    stp_port->state = STP_PORT_STATE_FORWARDING;
                    stp_send_bpdu(intf); // Send our BPDU to claim designated role
                }
            }
        }
    } else if (compare_bridge_id(bpdu->bridge_id, stp->bridge_id) == 0) {
        // BPDU from ourselves - ignore
        return;
    } else {
        // Same root or inferior root - need to determine designated port
        // Compare path costs to root
        unsigned int our_path_cost = stp->root_path_cost;
        unsigned int their_path_cost = bpdu->root_path_cost + stp_port->path_cost;
        
        if (their_path_cost < our_path_cost) {
            // Their path is better - we should block this port
            stp_port->role = STP_PORT_ROLE_ALTERNATE;
            stp_port->state = STP_PORT_STATE_BLOCKING;
            // Send BPDU on other ports to notify neighbors
            unsigned int i;
            for (i = 0; i < MAX_INTF_PER_NODE; i++) {
                if (stp->stp_ports[i] && stp->stp_ports[i] != stp_port && 
                    stp->stp_ports[i]->intf->link) {
                    stp_send_bpdu(stp->stp_ports[i]->intf);
                }
            }
        } else if (their_path_cost > our_path_cost) {
            // Our path is better - we should be designated
            stp_port->role = STP_PORT_ROLE_DESIGNATED;
            stp_port->state = STP_PORT_STATE_FORWARDING;
            // Send our BPDU to show we're designated
            stp_send_bpdu(intf);
        } else {
            // Same path cost - compare bridge IDs
            int bridge_cmp = compare_bridge_id(bpdu->bridge_id, stp->bridge_id);
            if (bridge_cmp < 0) {
                // Their bridge ID is lower - they should be designated, we block
                stp_port->role = STP_PORT_ROLE_ALTERNATE;
                stp_port->state = STP_PORT_STATE_BLOCKING;
                // Send BPDU on other ports to notify neighbors
                unsigned int i;
                for (i = 0; i < MAX_INTF_PER_NODE; i++) {
                    if (stp->stp_ports[i] && stp->stp_ports[i] != stp_port && 
                        stp->stp_ports[i]->intf->link) {
                        stp_send_bpdu(stp->stp_ports[i]->intf);
                    }
                }
            } else {
                // Our bridge ID is lower - we should be designated
                stp_port->role = STP_PORT_ROLE_DESIGNATED;
                stp_port->state = STP_PORT_STATE_FORWARDING;
                // Send our BPDU to show we're designated
                stp_send_bpdu(intf);
            }
        }
    }
}

void stp_send_bpdu(interface_t *intf) {
    stp_t *stp = get_stp_instance(intf->att_node);
    if (!stp) return;
    
    stp_port_t *stp_port = get_stp_port(intf);
    if (!stp_port) return;
    
    // Send BPDU on all ports initially (for convergence), or on forwarding/designated ports
    // Blocked alternate ports should not send BPDUs (they're not designated)
    if (stp_port->state == STP_PORT_STATE_BLOCKING && 
        stp_port->role == STP_PORT_ROLE_ALTERNATE) {
        // Don't send BPDU on blocked alternate ports
        return;
    }
    
    ethernetHeader_t *eth_hdr = calloc(1, sizeof(ethernetHeader_t) + sizeof(stp_bpdu_t));
    memcpy(eth_hdr->dest.mac_address, stp_multicast_mac, sizeof(mac_address_t));
    memcpy(eth_hdr->src.mac_address, INTERFACE_MAC(intf), sizeof(mac_address_t));
    eth_hdr->type = STP_ETHER_TYPE;
    
    stp_bpdu_t *bpdu = (stp_bpdu_t *)eth_hdr->payload;
    bpdu->protocol_id = htons(STP_BPDU_PROTOCOL_ID);
    bpdu->version = STP_BPDU_VERSION;
    bpdu->bpdu_type = STP_BPDU_TYPE_CONFIG;
    bpdu->flags = 0;
    bpdu->root_bridge_id = stp_htonll(stp->root_bridge_id);
    bpdu->root_path_cost = htonl(stp->root_path_cost);
    bpdu->bridge_id = stp_htonll(stp->bridge_id);
    bpdu->port_id = htons(stp_port->port_id);
    bpdu->message_age = htons(0);
    bpdu->max_age = htons(20);
    bpdu->hello_time = htons(2);
    bpdu->forward_delay = htons(15);
    
    unsigned int pkt_size = sizeof(ethernetHeader_t) + sizeof(stp_bpdu_t);
    send_packet_out((char *)eth_hdr, pkt_size, intf);
    free(eth_hdr);
}

void stp_start(node_t *node) {
    stp_t *stp = get_stp_instance(node);
    if (!stp) {
        stp_init(node);
        stp = get_stp_instance(node);
    }
    
    // Initially, each switch thinks it's the root
    // BPDU exchange will determine the actual root bridge
    stp->is_root_bridge = TRUE;
    stp->root_bridge_id = stp->bridge_id;
    stp->root_path_cost = 0;
    
    // Set all ports to designated/forwarding initially
    // They will be blocked if better BPDUs are received
    unsigned int i;
    for (i = 0; i < MAX_INTF_PER_NODE; i++) {
        if (stp->stp_ports[i]) {
            stp->stp_ports[i]->role = STP_PORT_ROLE_DESIGNATED;
            stp->stp_ports[i]->state = STP_PORT_STATE_FORWARDING;
        }
    }
    
    // Send initial BPDUs on all ports to start election
    // Send one round initially - more will be sent as needed
    for (i = 0; i < MAX_INTF_PER_NODE; i++) {
        if (stp->stp_ports[i] && stp->stp_ports[i]->intf->link) {
            stp_send_bpdu(stp->stp_ports[i]->intf);
        }
    }
}

void stp_stop(node_t *node) {
    stp_t *stp = get_stp_instance(node);
    if (!stp) return;
    
    // Set all ports to forwarding (disable STP)
    unsigned int i;
    for (i = 0; i < MAX_INTF_PER_NODE; i++) {
        if (stp->stp_ports[i]) {
            stp->stp_ports[i]->state = STP_PORT_STATE_FORWARDING;
        }
    }
}

void stp_dump(node_t *node) {
    stp_t *stp = get_stp_instance(node);
    if (!stp) {
        printf("STP not initialized on node %s\n", node->node_name);
        return;
    }
    
    printf("\n=== STP Status for Node: %s ===\n", node->node_name);
    printf("Bridge ID: 0x%016llx\n", stp->bridge_id);
    printf("Root Bridge ID: 0x%016llx\n", stp->root_bridge_id);
    printf("Root Path Cost: %u\n", stp->root_path_cost);
    printf("Is Root Bridge: %s\n", stp->is_root_bridge ? "Yes" : "No");
    if (stp->root_port) {
        printf("Root Port: %s\n", stp->root_port->if_name);
    }
    
    printf("\nPort Status:\n");
    printf("%-15s %-12s %-12s %-10s\n", "Interface", "State", "Role", "Cost");
    printf("------------------------------------------------------------\n");
    
    unsigned int i;
    for (i = 0; i < MAX_INTF_PER_NODE; i++) {
        if (stp->stp_ports[i]) {
            stp_port_t *port = stp->stp_ports[i];
            const char *state_str[] = {"Disabled", "Listening", "Learning", "Forwarding", "Blocking"};
            const char *role_str[] = {"Root", "Designated", "Alternate", "Backup"};
            
            printf("%-15s %-12s %-12s %-10u\n",
                port->intf->if_name,
                state_str[port->state],
                role_str[port->role],
                port->path_cost);
        }
    }
    printf("\n");
}

void stp_block_port(interface_t *intf) {
    stp_port_t *stp_port = get_stp_port(intf);
    if (!stp_port) return;
    
    stp_port->state = STP_PORT_STATE_BLOCKING;
    stp_port->role = STP_PORT_ROLE_ALTERNATE;
    printf("STP: Blocked port %s on node %s\n", intf->if_name, intf->att_node->node_name);
}

void stp_unblock_port(interface_t *intf) {
    stp_port_t *stp_port = get_stp_port(intf);
    if (!stp_port) return;
    
    stp_port->state = STP_PORT_STATE_FORWARDING;
    stp_port->role = STP_PORT_ROLE_DESIGNATED;
    printf("STP: Unblocked port %s on node %s\n", intf->if_name, intf->att_node->node_name);
}

// Helper function for network byte order conversion (renamed to avoid conflict with system macros)
static unsigned long long stp_ntohll(unsigned long long value) {
    unsigned long long result = 0;
    unsigned char *bytes = (unsigned char *)&value;
    result |= ((unsigned long long)bytes[0] << 56);
    result |= ((unsigned long long)bytes[1] << 48);
    result |= ((unsigned long long)bytes[2] << 40);
    result |= ((unsigned long long)bytes[3] << 32);
    result |= ((unsigned long long)bytes[4] << 24);
    result |= ((unsigned long long)bytes[5] << 16);
    result |= ((unsigned long long)bytes[6] << 8);
    result |= ((unsigned long long)bytes[7]);
    return result;
}

static unsigned long long stp_htonll(unsigned long long value) {
    return stp_ntohll(value); // Same operation
}
