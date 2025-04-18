#include "ip.h"
#include "arp.h"
#include "ethernet.h"
#include "../string.h"

static uint32_t our_ip_addr = 0;
static uint16_t ip_id = 0;
static ip_receive_callback_t protocol_handlers[256] = {0};

// Calculate IP checksum
static uint16_t ip_checksum(const void* data, size_t length) {
    const uint16_t* ptr = (const uint16_t*)data;
    uint32_t sum = 0;

    while (length > 1) {
        sum += *ptr++;
        length -= 2;
    }

    if (length > 0) {
        sum += *(const uint8_t*)ptr;
    }

    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }

    return ~sum;
}

void ip_init(uint32_t our_ip) {
    our_ip_addr = our_ip;
}

bool ip_send_packet(uint32_t dest_ip, uint8_t protocol, const void* data, uint16_t length) {
    uint8_t packet[1500];  // MTU size
    ip_header_t* ip = (ip_header_t*)packet;

    // Fill IP header
    ip->version_ihl = 0x45;  // IPv4, 5 DWORDS header length
    ip->tos = 0;
    ip->total_length = __builtin_bswap16(sizeof(ip_header_t) + length);
    ip->id = __builtin_bswap16(ip_id++);
    ip->flags_fragment = 0;
    ip->ttl = 64;
    ip->protocol = protocol;
    ip->checksum = 0;
    ip->src_ip = our_ip_addr;
    ip->dest_ip = dest_ip;

    // Copy payload
    memcpy(ip->payload, data, length);

    // Calculate checksum
    ip->checksum = ip_checksum(ip, sizeof(ip_header_t));

    // Look up destination MAC address
    uint8_t dest_mac[6];
    if (!arp_lookup(dest_ip, dest_mac)) {
        // Send ARP request and wait for reply
        arp_send_request(dest_ip);
        // In a real implementation, we would queue the packet and send it when we get the ARP reply
        return false;
    }

    // Send packet
    return ethernet_send_frame(dest_mac, ETH_TYPE_IP, packet, sizeof(ip_header_t) + length);
}

void ip_register_protocol_handler(uint8_t protocol, ip_receive_callback_t callback) {
    protocol_handlers[protocol] = callback;
}

// Handle received IP packets
static void ip_receive(const eth_frame_t* frame, uint16_t length) {
    const ip_header_t* ip = (const ip_header_t*)frame->payload;

    // Basic validation
    if (length < sizeof(ip_header_t)) return;
    if ((ip->version_ihl >> 4) != 4) return;  // IPv4 only

    uint16_t total_length = __builtin_bswap16(ip->total_length);
    if (total_length > length) return;

    // Check if packet is for us
    if (ip->dest_ip != our_ip_addr) return;

    // Call protocol handler if registered
    if (protocol_handlers[ip->protocol]) {
        protocol_handlers[ip->protocol](ip, total_length);
    }
}
