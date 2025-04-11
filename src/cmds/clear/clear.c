#include "../command_registry.h"
#include "../../libs/print.h"

// Function to execute the clear command
static void CMD_clear(const char* args) {
    print_clear();
}

// Define the clear command
static const command_t clear_command = {
    .name = "clear",
    .short_desc = "Clear the terminal screen",
    .usage = "clear",
    .long_desc = "Clears the terminal screen and resets the cursor to the top-left corner.",
    .examples = "clear",
    .execute = CMD_clear
};

// Initialization function for the clear command
void CMD_init_clear() {
    register_command(&clear_command);
}