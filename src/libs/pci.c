#include "pci.h"
#include "port.h"

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA    0xCFC

static uint32_t pci_read_config(uint8_t bus, uint8_t device, uint8_t func, uint8_t offset) {
    uint32_t address = (uint32_t)((bus << 16) | (device << 11) |
                                 (func << 8) | (offset & 0xFC) | 0x80000000);
    port_dword_out(PCI_CONFIG_ADDRESS, address);
    return port_dword_in(PCI_CONFIG_DATA);
}

static void pci_write_config(uint8_t bus, uint8_t device, uint8_t func,
                           uint8_t offset, uint32_t value) {
    uint32_t address = (uint32_t)((bus << 16) | (device << 11) |
                                 (func << 8) | (offset & 0xFC) | 0x80000000);
    port_dword_out(PCI_CONFIG_ADDRESS, address);
    port_dword_out(PCI_CONFIG_DATA, value);
}

bool pci_find_device(uint16_t vendor_id, uint16_t device_id, pci_device_t* device) {
    for (uint16_t bus = 0; bus < 256; bus++) {
        for (uint16_t dev = 0; dev < 32; dev++) {
            for (uint16_t func = 0; func < 8; func++) {
                uint32_t config = pci_read_config(bus, dev, func, 0);

                if ((config & 0xFFFF) == vendor_id) {
                    if (((config >> 16) & 0xFFFF) == device_id) {
                        device->bus = bus;
                        device->device = dev;
                        device->function = func;
                        device->vendor_id = vendor_id;
                        device->device_id = device_id;

                        // Read class information
                        uint32_t class_info = pci_read_config(bus, dev, func, 0x08);
                        device->revision_id = class_info & 0xFF;
                        device->prog_if = (class_info >> 8) & 0xFF;
                        device->subclass = (class_info >> 16) & 0xFF;
                        device->class_code = (class_info >> 24) & 0xFF;

                        // Read BARs
                        for (int i = 0; i < 6; i++) {
                            device->bar[i] = pci_read_config(bus, dev, func, 0x10 + (i * 4));
                        }

                        return true;
                    }
                }
            }
        }
    }
    return false;
}

void pci_enable_bus_mastering(pci_device_t* device) {
    uint32_t command = pci_read_config(device->bus, device->device, device->function, 0x04);
    command |= (1 << 2);  // Enable Bus Mastering
    command |= (1 << 0);  // Enable I/O Space
    command |= (1 << 1);  // Enable Memory Space
    pci_write_config(device->bus, device->device, device->function, 0x04, command);
}

uint64_t pci_map_bar(pci_device_t* device, int bar_num) {
    uint32_t bar = device->bar[bar_num];

    // Check if this is a memory BAR
    if (!(bar & 0x1)) {
        // Get the base address
        uint64_t base = bar & ~0xF;

        // Check if this is a 64-bit BAR
        if ((bar & 0x6) == 0x4) {
            base |= ((uint64_t)device->bar[bar_num + 1] << 32);
        }

        return base;
    }

    return 0;
}
