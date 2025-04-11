#include "../../libs/port.h"
#include "../../libs/print.h"
#include "../../libs/timer.h"
#include "../../libs/interrupt.h"
#include "reboot.h"
#include "../../kernel/panic.h"
#include "../command_registry.h"

void CMD_reboot() {
    print_str("Rebooting femboyOS...\n");
    sleep(1000);

    //Keyboard controller
    uint8_t good = 0x02;
    while (good & 0x02)
        good = port_byte_in(0x64);
    port_byte_out(0x64, 0xFE);

    //Triple fault
    disable_interrupts();

    // Load invalid IDT to force triple fault
    idtr_t zero_idt = {0, 0};
    __asm__ volatile("lidt %0" : : "m"(zero_idt));

    // Trigger interrupt
    __asm__ volatile("int $0x03");

    PANIC("Failed to rebbot");
}

command_t CMD_reboot_command = {
    .name = "reboot",
    .short_desc = "Reboot the system",
    .usage = "reboot",
    .long_desc = "Reboots the system.",
    .examples = "reboot",
    .execute = CMD_reboot
};

void CMD_init_reboot() {
    register_command(&CMD_reboot_command);
}