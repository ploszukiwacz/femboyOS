#include "ethernet.h"
#include "e1000.h"
#include "../interrupt.h"
#include "../print.h"
#include "../timer.h"
#include "../types.h"
#include "../string.h"  // Add this for memcpy

static eth_receive_callback_t receive_callback = NULL;
static uint8_t our_mac[6];

// Interrupt handler for received packets
static void ethernet_irq_handler(void) {
    uint8_t buffer[1518];  // Max ethernet frame size
    uint16_t length;

    while ((length = e1000_receive_packet(buffer, sizeof(buffer))) > 0) {
        if (receive_callback) {
            receive_callback((eth_frame_t*)buffer, length);
        }
    }
}

bool ethernet_init(void) {
    // Try to initialize the network card
    if (!e1000_init()) {
        return false;
    }

    // Get our MAC address
    e1000_get_mac_address(our_mac);

    // Print MAC address
    print_str("MAC Address: ");
    for (int i = 0; i < 6; i++) {
        print_hex(our_mac[i]);
        if (i < 5) print_str(":");
    }
    print_str("\n");

    // Register interrupt handler
    register_interrupt_handler(IRQ_NETWORK, ethernet_irq_handler);

    return true;
}

bool ethernet_send_frame(const uint8_t* dest_mac, uint16_t type,
                        const void* payload, uint16_t length) {
    uint8_t frame[1518];
    eth_frame_t* eth = (eth_frame_t*)frame;

    // Build ethernet header
    memcpy(eth->dest_mac, dest_mac, 6);
    memcpy(eth->src_mac, our_mac, 6);
    eth->type = (type >> 8) | (type << 8);  // Convert to network byte order

    // Copy payload
    memcpy(eth->payload, payload, length);

    // Send frame
    return e1000_send_packet(frame, length + sizeof(eth_frame_t));
}

void ethernet_register_callback(eth_receive_callback_t callback) {
    receive_callback = callback;
}
