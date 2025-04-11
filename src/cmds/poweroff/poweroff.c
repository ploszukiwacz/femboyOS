#include "../../libs/port.h"
#include "../../libs/print.h"
#include "../../libs/timer.h"
#include "poweroff.h"
#include "../../kernel/panic.h"
#include "../command_registry.h"

void CMD_poweroff() {
    print_str("Shutting down femboyOS...\n");
    sleep(1000);

    // ACPI shutdown
    port_word_out(0x604, 0x2000);

    // APM shutdown
    port_word_out(0xB004, 0x2000);

    // bochs/qemu specific shutdown
    port_word_out(0x8900, 0x5301);
    port_word_out(0x8900, 0x5307);
    port_word_out(0x8900, 0x530D);

    PANIC("Failed to power off");
}

command_t CMD_poweroff_command = {
    .name = "poweroff",
    .short_desc = "Shut down the system",
    .usage = "poweroff",
    .long_desc = "Shuts down the system gracefully.",
    .examples = "poweroff",
    .execute = CMD_poweroff
};

void CMD_init_poweroff() {
    register_command(&CMD_poweroff_command);
}