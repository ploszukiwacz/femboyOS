#include "../libs/print.h"
#include "../libs/random.h"

int CMD_random(const char* args) {
    uint8_t saved_color = color;

    // Skip command name
    while (*args && *args != ' ') args++;
    while (*args == ' ') args++;

    // No arguments - generate 0-100
    if (!*args) {
        uint32_t result = random_range(0, 100);
        print_str("Random number (0-100): ");
        print_number(result);
        print_str("\n");
        return 0;
    }

    // Parse min and max
    uint32_t min = 0;
    uint32_t max = 0;
    bool parsing_max = false;

    while (*args) {
        if (*args == ' ' || *args == ',') {
            parsing_max = true;
            args++;
            continue;
        }

        if (*args >= '0' && *args <= '9') {
            if (parsing_max) {
                max = max * 10 + (*args - '0');
            } else {
                min = min * 10 + (*args - '0');
            }
        }
        args++;
    }

    // If only min was provided, use it as max and 0 as min
    if (!parsing_max) {
        max = min;
        min = 0;
    }

    // Generate and display random number
    uint32_t result = random_range(min, max);
    print_str("Random number between ");
    print_number(min);
    print_str(" and ");
    print_number(max);
    print_str(": ");
    print_number(result);
    print_str("\n");

    print_set_color(saved_color, PRINT_COLOR_BLACK);
    return 0;
}
