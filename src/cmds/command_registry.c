#include "command_registry.h"
#include "../libs/print.h"

// Load commands
#include "echo/echo.h"
#include "help/help.h"
#include "man/man.h"
#include "dance/dance.h"
#include "poweroff/poweroff.h"
#include "reboot/reboot.h"
#include "random/random.h"
#include "fortune/fortune.h"
#include "hardtest/hardtest.h"
#include "keytest/keytest.h"
#include "clear/clear.h"

static command_t command_registry[MAX_COMMANDS];
static int command_count = 0;

static void (*command_init_functions[])() = {
    CMD_init_echo,
    CMD_init_help,
    CMD_init_man,
    CMD_init_dance,
    CMD_init_poweroff,
    CMD_init_reboot,
    CMD_init_random,
    CMD_init_fortune,
    CMD_init_hardtest,
    CMD_init_keytest,
    CMD_init_clear
};

void register_command(const command_t* cmd) {
    if (command_count < MAX_COMMANDS) {
        command_registry[command_count++] = *cmd;
    }
}

const command_t* get_registered_commands(int* count) {
    *count = command_count;
    return command_registry;
}

void initialize_command_registry() {
    for (int i = 0; i < sizeof(command_init_functions) / sizeof(command_init_functions[0]); i++) {
        command_init_functions[i]();
    }
}