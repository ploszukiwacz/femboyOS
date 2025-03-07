#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint8_t bus;
    uint8_t device;
    uint8_t function;
    uint16_t vendor_id;
    uint16_t device_id;
    uint8_t class_code;
    uint8_t subclass;
    uint8_t prog_if;
    uint8_t revision_id;
    uint32_t bar[6];
} pci_device_t;

bool pci_find_device(uint16_t vendor_id, uint16_t device_id, pci_device_t* device);
void pci_enable_bus_mastering(pci_device_t* device);
uint64_t pci_map_bar(pci_device_t* device, int bar_num);
