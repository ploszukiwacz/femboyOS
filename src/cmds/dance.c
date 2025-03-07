#include "../libs/keyboard.h"
#include "../libs/print.h"
#include "../libs/timer.h"
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

    // Consume the key that was pressed
    keyboard_read();

    print_str("\nDance party over!\n");
}
