#pragma once

#include <stdint.h>
#include "../types.h"

#define ICMP_ECHO_REQUEST 8
#define ICMP_ECHO_REPLY   0

typedef struct {
    uint8_t  type;
    uint8_t  code;
    uint16_t checksum;
    uint16_t identifier;
    uint16_t sequence;
    uint8_t  data[];
} __attribute__((packed)) icmp_header_t;

// Initialize ICMP subsystem
void icmp_init(void);

// Send an ICMP echo request (ping)
bool icmp_send_echo_request(uint32_t dest_ip, uint16_t sequence);
