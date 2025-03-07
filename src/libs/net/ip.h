#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "../types.h"

// IP Protocol numbers
#define IP_PROTOCOL_ICMP  1
#define IP_PROTOCOL_TCP   6
#define IP_PROTOCOL_UDP   17

// Maximum packet sizes
#define IP_MAX_PACKET_SIZE 65535
#define IP_HEADER_SIZE     20
#define IP_MAX_PAYLOAD     (IP_MAX_PACKET_SIZE - IP_HEADER_SIZE)

// IP header flags
#define IP_FLAG_DF (1 << 14)  // Don't Fragment
#define IP_FLAG_MF (1 << 13)  // More Fragments

// IP header structure
typedef struct {
    uint8_t  version_ihl;    // Version (4 bits) + Internet Header Length (4 bits)
    uint8_t  tos;            // Type of Service
    uint16_t total_length;   // Total Length
    uint16_t id;             // Identification
    uint16_t flags_fragment; // Flags (3 bits) + Fragment Offset (13 bits)
    uint8_t  ttl;            // Time To Live
    uint8_t  protocol;       // Protocol
    uint16_t checksum;       // Header Checksum
    uint32_t src_ip;         // Source IP Address
    uint32_t dest_ip;        // Destination IP Address
    uint8_t  payload[];      // Variable length payload
} __attribute__((packed)) ip_header_t;

// Helper macros for IP address manipulation
#define IP_ADDR(a,b,c,d) ((uint32_t)((a) | ((b) << 8) | ((c) << 16) | ((d) << 24)))
#define IP_ADDR_PART1(addr) (((addr) >> 0) & 0xFF)
#define IP_ADDR_PART2(addr) (((addr) >> 8) & 0xFF)
#define IP_ADDR_PART3(addr) (((addr) >> 16) & 0xFF)
#define IP_ADDR_PART4(addr) (((addr) >> 24) & 0xFF)

// Callback type for received packets
typedef void (*ip_receive_callback_t)(const ip_header_t* packet, uint16_t length);

// Initialize IP subsystem with our IP address
void ip_init(uint32_t our_ip);

// Get our IP address
uint32_t ip_get_address(void);

// Set our IP address
void ip_set_address(uint32_t ip);

// Send an IP packet
bool ip_send_packet(uint32_t dest_ip, uint8_t protocol, const void* data, uint16_t length);

// Register a callback for a specific protocol
void ip_register_protocol_handler(uint8_t protocol, ip_receive_callback_t callback);

// Convert string to IP address
uint32_t ip_str_to_addr(const char* str);

// Convert IP address to string (buffer should be at least 16 bytes)
void ip_addr_to_str(uint32_t addr, char* buffer);

// Check if an IP address is valid
bool ip_is_valid_address(uint32_t addr);

// Calculate IP checksum
uint16_t ip_calculate_checksum(const void* data, uint16_t length);

// Handle incoming packets (called by ethernet driver)
void ip_receive_packet(const void* data, uint16_t length);

// Network byte order conversion functions
static inline uint16_t htons(uint16_t x) {
    return (x >> 8) | (x << 8);
}

static inline uint16_t ntohs(uint16_t x) {
    return htons(x);
}

static inline uint32_t htonl(uint32_t x) {
    return ((x >> 24) & 0x000000FF) |
           ((x >>  8) & 0x0000FF00) |
           ((x <<  8) & 0x00FF0000) |
           ((x << 24) & 0xFF000000);
}

static inline uint32_t ntohl(uint32_t x) {
    return htonl(x);
}
