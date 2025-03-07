#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "e1000.h"  // For e1000_get_mac_address
#include "../string.h"  // For memcpy

bool arp_lookup(uint32_t ip, uint8_t mac[6]);

#define ARP_HTYPE_ETHERNET 1
#define ARP_PTYPE_IPV4    0x0800

#define ARP_OP_REQUEST    1
#define ARP_OP_REPLY      2

typedef struct {
    uint16_t htype;           // Hardware type
    uint16_t ptype;           // Protocol type
    uint8_t  hlen;            // Hardware address length
    uint8_t  plen;            // Protocol address length
    uint16_t oper;            // Operation
    uint8_t  sha[6];          // Sender hardware address
    uint32_t spa;             // Sender protocol address
    uint8_t  tha[6];          // Target hardware address
    uint32_t tpa;             // Target protocol address
} __attribute__((packed)) arp_packet_t;

// Initialize ARP subsystem
void arp_init(void);

// Send an ARP request
void arp_send_request(uint32_t target_ip);

// Update ARP cache with new mapping
void arp_update(uint32_t ip, const uint8_t mac[6]);

// Look up MAC address for IP
bool arp_lookup(uint32_t ip, uint8_t mac[6]);
