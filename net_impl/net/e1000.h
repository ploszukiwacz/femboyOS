#pragma once

#include <stdint.h>
#include <stdbool.h>

#define E1000_VENDOR_ID 0x8086  // Intel
#define E1000_DEVICE_ID 0x100E  // 82540EM Gigabit Ethernet Controller

// Initialize the network card
bool e1000_init(void);

// Send a packet
bool e1000_send_packet(const void* data, uint16_t length);

// Receive a packet (non-blocking)
// Returns number of bytes received, or 0 if no packet available
uint16_t e1000_receive_packet(void* buffer, uint16_t max_length);

// Get MAC address
void e1000_get_mac_address(uint8_t mac[6]);
