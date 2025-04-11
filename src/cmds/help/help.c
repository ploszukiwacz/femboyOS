#include "../command_registry.h"
#include "../../libs/print.h"

void CMD_help() {
    int command_count;
    const command_t* commands = get_registered_commands(&command_count);

    print_str("Available commands:\n");
    for (int i = 0; i < command_count; i++) {
        print_str("  ");
        print_str(commands[i].name);
        print_str(" - ");
        print_str(commands[i].short_desc);
        print_str("\n");
    }
    print_str("\nType 'man [command]' for more details about a command.\n");
}

static const command_t help_command = {
    .name = "help",
    .short_desc = "Display help information about available commands",
    .usage = "help",
    .long_desc = "Lists all available commands with a short description.\n"
                 "Use 'man [command]' for detailed information about a specific command.",
    .examples = "help\nhelp echo",
    .execute = CMD_help
};

void CMD_init_help() {
    register_command(&help_command);
}