#include "e1000.h"
#include "../pci.h"
#include "../memory.h"
#include "../port.h"
#include "../string.h"
#include "../timer.h"
#include "../print.h"

// E1000 Register offsets
#define REG_CTRL        0x0000
#define REG_STATUS      0x0008
#define REG_EEPROM      0x0014
#define REG_CTRL_EXT    0x0018
#define REG_ICR         0x00C0
#define REG_IMS         0x00D0
#define REG_RCTL        0x0100
#define REG_TCTL        0x0400
#define REG_RDBAL       0x2800
#define REG_RDBAH       0x2804
#define REG_RDLEN       0x2808
#define REG_RDH         0x2810
#define REG_RDT         0x2818
#define REG_TDBAL       0x3800
#define REG_TDBAH       0x3804
#define REG_TDLEN       0x3808
#define REG_TDH         0x3810
#define REG_TDT         0x3818
#define REG_MTA         0x5200
#define REG_RAL         0x5400
#define REG_RAH         0x5404

// Receive Descriptor status bits
#define RDES_DD        0x01    // Descriptor Done
#define RDES_EOP       0x02    // End of Packet

// Transmit Descriptor status bits
#define TDES_DD        0x01    // Descriptor Done
#define TDES_EOP       0x02    // End of Packet

// Number of receive/transmit descriptors
#define RX_DESC_COUNT  32
#define TX_DESC_COUNT  32
#define RX_BUFFER_SIZE 2048

// Descriptor structures
struct rx_desc {
    uint64_t addr;     // Buffer address
    uint16_t length;   // Length of received data
    uint16_t checksum; // Checksum
    uint8_t  status;   // Status bits
    uint8_t  errors;   // Error bits
    uint16_t special;
} __attribute__((packed));

struct tx_desc {
    uint64_t addr;     // Buffer address
    uint16_t length;   // Data length
    uint8_t  cso;      // Checksum offset
    uint8_t  cmd;      // Command bits
    uint8_t  status;   // Status bits
    uint8_t  css;      // Checksum start
    uint16_t special;
} __attribute__((packed));

// Driver state
static struct {
    uint64_t mmio_base;            // Memory-mapped I/O base address
    struct rx_desc* rx_descs;      // Receive descriptors
    struct tx_desc* tx_descs;      // Transmit descriptors
    void* rx_buffers[RX_DESC_COUNT];  // Receive buffers
    void* tx_buffers[TX_DESC_COUNT];  // Transmit buffers
    uint32_t rx_cur;               // Current receive descriptor
    uint32_t tx_cur;               // Current transmit descriptor
    uint8_t mac_addr[6];          // MAC address
} e1000;

// Read from MMIO register
static inline uint32_t e1000_read_reg(uint32_t reg) {
    return *(volatile uint32_t*)(e1000.mmio_base + reg);
}

// Write to MMIO register
static inline void e1000_write_reg(uint32_t reg, uint32_t value) {
    *(volatile uint32_t*)(e1000.mmio_base + reg) = value;
}

// Initialize receive descriptors
static void init_rx_desc(void) {
    // Allocate and initialize receive descriptors
    e1000.rx_descs = kalloc_aligned(RX_DESC_COUNT * sizeof(struct rx_desc), 16);
    memset(e1000.rx_descs, 0, RX_DESC_COUNT * sizeof(struct rx_desc));

    // Allocate receive buffers
    for (int i = 0; i < RX_DESC_COUNT; i++) {
        e1000.rx_buffers[i] = kalloc_aligned(RX_BUFFER_SIZE, 16);
        e1000.rx_descs[i].addr = (uint64_t)e1000.rx_buffers[i];
    }

    // Setup receive descriptor ring buffer
    e1000_write_reg(REG_RDBAL, (uint64_t)e1000.rx_descs & 0xFFFFFFFF);
    e1000_write_reg(REG_RDBAH, (uint64_t)e1000.rx_descs >> 32);
    e1000_write_reg(REG_RDLEN, RX_DESC_COUNT * sizeof(struct rx_desc));
    e1000_write_reg(REG_RDH, 0);
    e1000_write_reg(REG_RDT, RX_DESC_COUNT - 1);

    // Enable receiver
    uint32_t rctl = e1000_read_reg(REG_RCTL);
    rctl |= (1 << 1);  // Enable receiver
    rctl |= (1 << 4);  // Strip ethernet CRC
    e1000_write_reg(REG_RCTL, rctl);
}

// Initialize transmit descriptors
static void init_tx_desc(void) {
    // Allocate and initialize transmit descriptors
    e1000.tx_descs = kalloc_aligned(TX_DESC_COUNT * sizeof(struct tx_desc), 16);
    memset(e1000.tx_descs, 0, TX_DESC_COUNT * sizeof(struct tx_desc));

    // Setup transmit descriptor ring buffer
    e1000_write_reg(REG_TDBAL, (uint64_t)e1000.tx_descs & 0xFFFFFFFF);
    e1000_write_reg(REG_TDBAH, (uint64_t)e1000.tx_descs >> 32);
    e1000_write_reg(REG_TDLEN, TX_DESC_COUNT * sizeof(struct tx_desc));
    e1000_write_reg(REG_TDH, 0);
    e1000_write_reg(REG_TDT, 0);

    // Enable transmitter
    uint32_t tctl = e1000_read_reg(REG_TCTL);
    tctl |= (1 << 1);  // Enable transmitter
    tctl |= (1 << 3);  // Pad short packets
    e1000_write_reg(REG_TCTL, tctl);
}

// Read MAC address from EEPROM
static void read_mac_address(void) {
    uint32_t ral = e1000_read_reg(REG_RAL);
    uint32_t rah = e1000_read_reg(REG_RAH);

    e1000.mac_addr[0] = (ral >>  0) & 0xFF;
    e1000.mac_addr[1] = (ral >>  8) & 0xFF;
    e1000.mac_addr[2] = (ral >> 16) & 0xFF;
    e1000.mac_addr[3] = (ral >> 24) & 0xFF;
    e1000.mac_addr[4] = (rah >>  0) & 0xFF;
    e1000.mac_addr[5] = (rah >>  8) & 0xFF;
}

bool e1000_init(void) {
    // Find the E1000 PCI device
    pci_device_t device;
    if (!pci_find_device(E1000_VENDOR_ID, E1000_DEVICE_ID, &device)) {
        return false;
    }

    // Map the device's BAR0 (contains registers)
    e1000.mmio_base = pci_map_bar(&device, 0);
    if (!e1000.mmio_base) {
        return false;
    }


    // Try to read a known register to verify MMIO access works
        uint32_t test_read = e1000_read_reg(REG_STATUS);

        if (test_read == 0xFFFFFFFF) {
            // Invalid read - device not responding
            return false;
        }

        // Enable PCI bus mastering
        pci_enable_bus_mastering(&device);

        uint32_t timeout = 100;  // Reduced timeout
            uint32_t ctrl = e1000_read_reg(REG_CTRL);

            // Only try reset if we can read the control register
            if (ctrl != 0xFFFFFFFF) {
                e1000_write_reg(REG_CTRL, ctrl | (1 << 26));  // Set reset bit

                // Wait for reset to complete
                for (uint32_t i = 0; i < timeout && (e1000_read_reg(REG_CTRL) & (1 << 26)); i++) {
                    sleep(1);

                    // Break early if we're getting invalid reads
                    if (e1000_read_reg(REG_CTRL) == 0xFFFFFFFF) {
                        return false;
                    }
                }
            }


    // Reset the device
    e1000_write_reg(REG_CTRL, e1000_read_reg(REG_CTRL) | (1 << 26));
    for (int i = 0; i < 1000; i++) {
            print_str("meow");
        if (!(e1000_read_reg(REG_CTRL) & (1 << 26))) {
                print_str("meow");
            break;
        }
            print_str("meow");
        sleep(1);
    }

    // Initialize descriptors
    init_rx_desc();
    init_tx_desc();

    // Read MAC address
    read_mac_address();

    return true;
}

bool e1000_send_packet(const void* data, uint16_t length) {
    // Get next transmit descriptor
    struct tx_desc* desc = &e1000.tx_descs[e1000.tx_cur];

    // Check if descriptor is available
    if (desc->status & TDES_DD) {
        // Copy data to transmit buffer
        if (e1000.tx_buffers[e1000.tx_cur] == NULL) {
            e1000.tx_buffers[e1000.tx_cur] = kalloc_aligned(length, 16);
        }
        memcpy(e1000.tx_buffers[e1000.tx_cur], data, length);

        // Setup descriptor
        desc->addr = (uint64_t)e1000.tx_buffers[e1000.tx_cur];
        desc->length = length;
        desc->cmd = (1 << 0) | (1 << 1);  // EOP and IFCS
        desc->status = 0;

        // Advance ring buffer
        e1000.tx_cur = (e1000.tx_cur + 1) % TX_DESC_COUNT;
        e1000_write_reg(REG_TDT, e1000.tx_cur);

        return true;
    }

    return false;
}

uint16_t e1000_receive_packet(void* buffer, uint16_t max_length) {
    // Get next receive descriptor
    struct rx_desc* desc = &e1000.rx_descs[e1000.rx_cur];

    // Check if packet is available
    if (desc->status & RDES_DD) {
        uint16_t length = desc->length;
        if (length > max_length) {
            length = max_length;
        }

        // Copy data to buffer
        memcpy(buffer, e1000.rx_buffers[e1000.rx_cur], length);

        // Reset descriptor
        desc->status = 0;

        // Advance ring buffer
        e1000.rx_cur = (e1000.rx_cur + 1) % RX_DESC_COUNT;
        e1000_write_reg(REG_RDT, e1000.rx_cur);

        return length;
    }

    return 0;
}

void e1000_get_mac_address(uint8_t mac[6]) {
    memcpy(mac, e1000.mac_addr, 6);
}
