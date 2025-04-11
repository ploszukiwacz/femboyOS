#include "man.h"
#include "../../libs/print.h"
#include "../../libs/string.h"
#include "../command_registry.h"

// Function to display a specific command's manual page
void CMD_man(const char* command) {
    if (command == NULL || *command == '\0') {
        print_str("Usage: man [command]\n");
        print_str("Type 'help' to see a list of available commands.\n");
        return;
    }

    int command_count;
    const command_t* commands = get_registered_commands(&command_count);

    for (int i = 0; i < command_count; i++) {
        if (strcmp(commands[i].name, command) == 0) {
            print_str("NAME\n    ");
            print_str(commands[i].name);
            print_str(" - ");
            print_str(commands[i].short_desc);
            print_str("\n\n");

            print_str("SYNOPSIS\n    ");
            print_str(commands[i].usage);
            print_str("\n\n");

            print_str("DESCRIPTION\n    ");
            print_str(commands[i].long_desc);
            print_str("\n\n");

            print_str("EXAMPLES\n    ");
            print_str(commands[i].examples);
            print_str("\n");
            return;
        }
    }

    print_str("No manual entry for ");
    print_str(command);
    print_str("\n");
}

static const command_t man_command = {
    .name = "man",
    .short_desc = "Manual page viewer",
    .usage = "man [command]",
    .long_desc = "Displays detailed documentation for the specified command.\n"
                 "Use 'man' without arguments to see a list of all documented commands.",
    .examples = "man echo\nman help",
    .execute = CMD_man
};

void CMD_init_man() {
    register_command(&man_command);
}