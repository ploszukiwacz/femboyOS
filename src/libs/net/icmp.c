#include "icmp.h"
#include "ip.h"
#include "../string.h"
#include "../print.h"

static uint16_t icmp_checksum(const void* data, size_t length) {
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

static void icmp_receive(const ip_header_t* ip, uint16_t length) {
    const icmp_header_t* icmp = (const icmp_header_t*)ip->payload;

    if (length < sizeof(icmp_header_t)) return;

    if (icmp->type == ICMP_ECHO_REQUEST) {
        // Prepare echo reply
        uint8_t reply[1500];
        icmp_header_t* reply_icmp = (icmp_header_t*)reply;

        // Copy original ICMP packet
        uint16_t icmp_length = length - sizeof(ip_header_t);
        memcpy(reply, icmp, icmp_length);

        // Modify for reply
        reply_icmp->type = ICMP_ECHO_REPLY;
        reply_icmp->checksum = 0;
        reply_icmp->checksum = icmp_checksum(reply, icmp_length);

        // Send reply
        ip_send_packet(ip->src_ip, IP_PROTOCOL_ICMP, reply, icmp_length);
    }
    else if (icmp->type == ICMP_ECHO_REPLY) {
        // Print received ping reply
        print_str("Ping reply from ");
        print_ip(ip->src_ip);
        print_str(": seq=");
        print_number(__builtin_bswap16(icmp->sequence));
        print_str("\n");
    }
}

void icmp_init(void) {
    ip_register_protocol_handler(IP_PROTOCOL_ICMP, icmp_receive);
}

bool icmp_send_echo_request(uint32_t dest_ip, uint16_t sequence) {
    uint8_t packet[64];
    icmp_header_t* icmp = (icmp_header_t*)packet;

    // Fill ICMP header
    icmp->type = ICMP_ECHO_REQUEST;
    icmp->code = 0;
    icmp->checksum = 0;
    icmp->identifier = __builtin_bswap16(0x1234);  // Any identifier
    icmp->sequence = __builtin_bswap16(sequence);

    // Add some data
    for (int i = 0; i < 56; i++) {
        icmp->data[i] = i;
    }

    // Calculate checksum
    icmp->checksum = icmp_checksum(icmp, sizeof(icmp_header_t) + 56);

    // Send packet
    return ip_send_packet(dest_ip, IP_PROTOCOL_ICMP, packet, sizeof(icmp_header_t) + 56);
}
