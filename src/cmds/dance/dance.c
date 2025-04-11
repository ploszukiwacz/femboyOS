#include "../../libs/keyboard.h"
#include "../../libs/print.h"
#include "../../libs/timer.h"
#include "../command_registry.h"
#include "dance.h"

void CMD_dance() {
    const char* dance_frames[] = {
        "(>^-^)>",
        "<(^-^<)",
        "^(^-^)^",
        "v(^-^)v",
        "<(^-^)>",
        "\\(^-^)/",
        "(>^-^)>",
        "<(^-^<)"
    };

    print_str("Press any key to stop the dance...\n");
    sleep(1000);

    for (int i = 0; !keyboard_is_key_available(); i++) {
        print_char('\r');
        print_str(dance_frames[i % 8]);
        sleep(200);
    }

    keyboard_read();

    print_str("\nDance party over!\n");
}

command_t CMD_dance_command = {
    .name = "dance",
    .short_desc = "Make the terminal dance.",
    .usage = "dance",
    .long_desc = "This command makes the terminal dance with ASCII art.",
    .examples = "dance\n",
    .execute = CMD_dance
};

void CMD_init_dance() {
    register_command(&CMD_dance_command);
}