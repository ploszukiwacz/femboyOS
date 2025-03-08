#include "../libs/port.h"
#include "../libs/print.h"
#include "../libs/timer.h"
#include "../libs/interrupt.h"
#include "restart.h"
#include "../kernel/panic.h"

void CMD_restart() {
    print_str("Rebooting femboyOS...\n");
    sleep(1000);

    // Method 1: Keyboard controller
    uint8_t good = 0x02;
    while (good & 0x02)
        good = port_byte_in(0x64);
    port_byte_out(0x64, 0xFE);

    // Method 2: Triple fault (if keyboard controller method fails)
    // print_str("Keyboard controller reboot failed. Trying triple fault...\n");
    // sleep(500);

    // Disable interrupts
    disable_interrupts();

    // Load invalid IDT to force triple fault
    idtr_t zero_idt = {0, 0};
    __asm__ volatile("lidt %0" : : "m"(zero_idt));

    // Trigger interrupt
    __asm__ volatile("int $0x03");

    PANIC("Failed to rebbot");
    // If we get here, both reboot methods failed
    // print_str("Failed to reboot. System halted.\n");
    // __asm__ volatile("cli; hlt");
}
