#include "../command_registry.h"
#include "../../libs/print.h"
#include "echo.h"

static void CMD_echo(const char* args) {
    print_str(args);
    print_str("\n");
}

static const command_t echo_command = {
    .name = "echo",
    .short_desc = "Display a line of text",
    .usage = "echo [text]",
    .long_desc = "Displays the specified text on the terminal.",
    .examples = "echo Hello, World!\necho This is femboyOS",
    .execute = CMD_echo
};

void CMD_init_echo() {
    register_command(&echo_command);
}