#include "../libs/keyboard.h"
#include "../libs/print.h"
#include "keytest.h"

void CMD_keytest() {
    print_str("Keyboard test mode. Press keys to see their values.\n");
    print_str("Press ESC three times to exit.\n\n");

    // Save current color
    uint8_t saved_color = color;
    int esc_count = 0;

    while (esc_count < 3) {
        char key = keyboard_read();

        if (key == 0x1B) {
            esc_count++;
            print_str("<ESC> ");

            if (keyboard_is_key_available()) {
                char second = keyboard_read();
                print_str("Sequence: ");
                print_char(second);
                print_str(" ");

                if (second == '[' && keyboard_is_key_available()) {
                    char third = keyboard_read();
                    print_char(third);
                    print_str(" ");

                    if ((third == '1' || third == '4' || third == '3' ||
                         third == '2' || third == '5' || third == '6') &&
                        keyboard_is_key_available()) {
                        char fourth = keyboard_read();
                        print_char(fourth);
                    }
                }

                esc_count = 0;  // Only count consecutive ESCs
            }

            print_str("\n");
        } else {
            esc_count = 0;

            print_str("Key: '");
            if (key >= ' ' && key <= '~') {
                print_char(key);
            } else {
                print_str("(non-printable)");
            }
            print_str("' Code: 0x");

            char hex_chars[] = "0123456789ABCDEF";
            print_char(hex_chars[(key >> 4) & 0xF]);
            print_char(hex_chars[key & 0xF]);
            print_str("\n");
        }
    }

    // Restore color and print exit message
    print_set_color(saved_color, PRINT_COLOR_BLACK);
    print_str("\nExiting keyboard test mode.\n");
}
