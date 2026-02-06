#ifndef __STP__
#define __STP__

#include "graph.h"
#include "gluethread/glthread.h"
#include "utils.h"
#include "tcpconst.h"

#define STP_BPDU_PROTOCOL_ID 0x0000
#define STP_BPDU_VERSION 0x00
#define STP_BPDU_TYPE_CONFIG 0x00
#define STP_BPDU_TYPE_TCN 0x80

#define STP_DEFAULT_BRIDGE_PRIORITY 32768
#define STP_DEFAULT_PORT_PRIORITY 128
#define STP_DEFAULT_COST 19  // For 100 Mbps

typedef enum {
    STP_PORT_STATE_DISABLED = 0,
    STP_PORT_STATE_LISTENING = 1,
    STP_PORT_STATE_LEARNING = 2,
    STP_PORT_STATE_FORWARDING = 3,
    STP_PORT_STATE_BLOCKING = 4
} stp_port_state_t;

typedef enum {
    STP_PORT_ROLE_ROOT = 0,
    STP_PORT_ROLE_DESIGNATED = 1,
    STP_PORT_ROLE_ALTERNATE = 2,
    STP_PORT_ROLE_BACKUP = 3
} stp_port_role_t;

#pragma pack(push, 1)
typedef struct stp_bpdu_ {
    unsigned short protocol_id;      // 0x0000
    unsigned char version;            // 0x00
    unsigned char bpdu_type;          // 0x00 = Config, 0x80 = TCN
    unsigned char flags;
    unsigned long long root_bridge_id;  // 8 bytes
    unsigned int root_path_cost;     // 4 bytes
    unsigned long long bridge_id;      // 8 bytes
    unsigned short port_id;           // 2 bytes
    unsigned short message_age;        // 2 bytes
    unsigned short max_age;           // 2 bytes
    unsigned short hello_time;        // 2 bytes
    unsigned short forward_delay;     // 2 bytes
} stp_bpdu_t;
#pragma pack(pop)

typedef struct stp_port_ {
    interface_t *intf;
    stp_port_state_t state;
    stp_port_role_t role;
    unsigned short port_id;
    unsigned int path_cost;
    bool_t is_edge_port;
    glthread_t stp_port_glue;
} stp_port_t;

typedef struct stp_ {
    node_t *node;
    unsigned long long bridge_id;
    unsigned long long root_bridge_id;
    unsigned int root_path_cost;
    interface_t *root_port;  // Interface leading to root
    stp_port_t *stp_ports[MAX_INTF_PER_NODE];
    glthread_t stp_port_list;
    bool_t is_root_bridge;
} stp_t;

GLTHREAD_TO_STRUCT(stp_port_glue_to_stp_port, stp_port_t, stp_port_glue);

/* STP API */
void stp_init(node_t *node);
void stp_start(node_t *node);
void stp_stop(node_t *node);
stp_t *get_stp_instance(node_t *node);
stp_port_t *get_stp_port(interface_t *intf);
bool_t stp_is_port_forwarding(interface_t *intf);
void stp_process_bpdu(interface_t *intf, void *bpdu);
void stp_send_bpdu(interface_t *intf);
void stp_dump(node_t *node);
void stp_block_port(interface_t *intf);
void stp_unblock_port(interface_t *intf);

#endif /* __STP__ */
