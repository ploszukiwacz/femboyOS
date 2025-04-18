#include "arp.h"
#include "ethernet.h"
#include "../string.h"

#define ARP_CACHE_SIZE 16

typedef struct {
    uint32_t ip;
    uint8_t mac[6];
    uint32_t time;
    bool valid;
} arp_entry_t;

static arp_entry_t arp_cache[ARP_CACHE_SIZE];
static uint32_t our_ip = 0;  // Will be set by DHCP later

static void arp_receive(const eth_frame_t* frame, uint16_t length) {
    if (length < sizeof(eth_frame_t) + sizeof(arp_packet_t)) return;

    const arp_packet_t* arp = (const arp_packet_t*)frame->payload;

    // Check if this is IPv4 over Ethernet
    if (arp->htype != ARP_HTYPE_ETHERNET ||
        arp->ptype != ARP_PTYPE_IPV4 ||
        arp->hlen != 6 ||
        arp->plen != 4) {
        return;
    }

    // Update ARP cache with sender's information
    arp_update(arp->spa, arp->sha);

    // If this is a request for our IP, send a reply
    if (arp->oper == ARP_OP_REQUEST && arp->tpa == our_ip) {
        uint8_t reply[sizeof(arp_packet_t)];
        arp_packet_t* reply_arp = (arp_packet_t*)reply;

        reply_arp->htype = ARP_HTYPE_ETHERNET;
        reply_arp->ptype = ARP_PTYPE_IPV4;
        reply_arp->hlen = 6;
        reply_arp->plen = 4;
        reply_arp->oper = ARP_OP_REPLY;

        uint8_t our_mac[6];
        e1000_get_mac_address(our_mac);

        memcpy(reply_arp->sha, our_mac, 6);
        reply_arp->spa = our_ip;
        memcpy(reply_arp->tha, arp->sha, 6);
        reply_arp->tpa = arp->spa;

        ethernet_send_frame(arp->sha, ETH_TYPE_ARP, reply, sizeof(arp_packet_t));
    }
}

void arp_init(void) {
    memset(arp_cache, 0, sizeof(arp_cache));
    ethernet_register_callback(arp_receive);
}

void arp_send_request(uint32_t target_ip) {
    uint8_t request[sizeof(arp_packet_t)];
    arp_packet_t* arp = (arp_packet_t*)request;

    arp->htype = ARP_HTYPE_ETHERNET;
    arp->ptype = ARP_PTYPE_IPV4;
    arp->hlen = 6;
    arp->plen = 4;
    arp->oper = ARP_OP_REQUEST;

    uint8_t our_mac[6];
    e1000_get_mac_address(our_mac);

    memcpy(arp->sha, our_mac, 6);
    arp->spa = our_ip;
    memset(arp->tha, 0, 6);
    arp->tpa = target_ip;

    uint8_t broadcast[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    ethernet_send_frame(broadcast, ETH_TYPE_ARP, request, sizeof(arp_packet_t));
}

void arp_update(uint32_t ip, const uint8_t mac[6]) {
    // Find existing entry or empty slot
    int empty_slot = -1;
    for (int i = 0; i < ARP_CACHE_SIZE; i++) {
        if (arp_cache[i].valid && arp_cache[i].ip == ip) {
            // Update existing entry
            memcpy(arp_cache[i].mac, mac, 6);
            arp_cache[i].time = 0;  // Reset age
            return;
        }
        if (!arp_cache[i].valid && empty_slot == -1) {
            empty_slot = i;
        }
    }

    // Add new entry if we found an empty slot
    if (empty_slot != -1) {
        arp_cache[empty_slot].ip = ip;
        memcpy(arp_cache[empty_slot].mac, mac, 6);
        arp_cache[empty_slot].time = 0;
        arp_cache[empty_slot].valid = true;
    }
}

bool arp_lookup(uint32_t ip, uint8_t mac[6]) {
    for (int i = 0; i < ARP_CACHE_SIZE; i++) {
        if (arp_cache[i].valid && arp_cache[i].ip == ip) {
            memcpy(mac, arp_cache[i].mac, 6);
            return true;
        }
    }
    return false;
}
