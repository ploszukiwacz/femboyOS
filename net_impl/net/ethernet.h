#pragma once

#include "../types.h"
#include "../print.h"

#define IRQ_NETWORK 11
#define ETH_TYPE_IP    0x0800
#define ETH_TYPE_ARP   0x0806

typedef struct {
    uint8_t  dest_mac[6];
    uint8_t  src_mac[6];
    uint16_t type;
    uint8_t  payload[];
} __attribute__((packed)) eth_frame_t;

// Initialize ethernet subsystem
bool ethernet_init(void);

// Send an ethernet frame
bool ethernet_send_frame(const uint8_t* dest_mac, uint16_t type,
                        const void* payload, uint16_t length);

// Register a callback for received frames
typedef void (*eth_receive_callback_t)(const eth_frame_t* frame, uint16_t length);
void ethernet_register_callback(eth_receive_callback_t callback);
