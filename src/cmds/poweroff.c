#include "../libs/port.h"
#include "../libs/print.h"
#include "../libs/timer.h"
#include "poweroff.h"

void CMD_poweroff() {
    print_str("Shutting down femboyOS...\n");
    sleep(1000);

    // Try ACPI shutdown
    port_word_out(0x604, 0x2000);  // Use port_word_out instead of port_byte_out

    // Try APM shutdown
    port_word_out(0xB004, 0x2000);  // Changed to port_word_out and correct value

    // Try bochs/qemu specific shutdown
    port_word_out(0x8900, 0x5301);
    port_word_out(0x8900, 0x5307);
    port_word_out(0x8900, 0x530D);

    // If we get here, shutdown failed
    print_str("Failed to power off. System halted.\n");
    __asm__ volatile("cli; hlt");
}
