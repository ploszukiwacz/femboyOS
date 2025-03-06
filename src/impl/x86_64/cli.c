#include "../../intf/cli.h"
#include "../../intf/print.h"
#include "../../intf/keyboard.h"
#include "../../intf/port.h"

#define CLI_MAX_COMMAND_LENGTH 256
#define CLI_HISTORY_SIZE 10

// External variables from print.c
extern size_t col;
extern size_t row;

// Command history
static char command_history[CLI_HISTORY_SIZE][CLI_MAX_COMMAND_LENGTH];
static int history_count = 0;
static int history_index = -1;

// Current command buffer
static char command_buffer[CLI_MAX_COMMAND_LENGTH];
static int buffer_position = 0;

// Forward declarations
static void cli_clear_line();
static void cli_prompt();
static void cli_display_command();
static int cli_strcmp(const char* str1, const char* str2);
static int cli_strncmp(const char* str1, const char* str2, int n); // Added declaration
static void cli_strncpy(char* dest, const char* src, int n);
static int cli_strlen(const char* str);
static void cli_handle_backspace();
static void cli_handle_up_arrow();
static void cli_handle_down_arrow();

void cli_init() {
    // Initialize the command buffer
    for (int i = 0; i < CLI_MAX_COMMAND_LENGTH; i++) {
        command_buffer[i] = 0;
    }

    // Initialize command history
    for (int i = 0; i < CLI_HISTORY_SIZE; i++) {
        for (int j = 0; j < CLI_MAX_COMMAND_LENGTH; j++) {
            command_history[i][j] = 0;
        }
    }

    // Print welcome message
    print_clear();
    print_set_color(PRINT_COLOR_LIGHT_CYAN, PRINT_COLOR_BLACK);
    print_str("femboyOS v0.1 ALPHA\n");
    print_set_color(PRINT_COLOR_LIGHT_GRAY, PRINT_COLOR_BLACK);
    print_str("PlOszukiwacz's Attempt at an os.\n\n");
    print_set_color(PRINT_COLOR_WHITE, PRINT_COLOR_BLACK);
    print_str("Type 'help' for a list of commands\n\n");
}

void cli_run() {
    cli_prompt();

    while (1) {
        // Read from I/O port to keep timer functioning
        port_byte_in(0x64);

        // Read a key from the keyboard
        char key = keyboard_read();

        // Handle special keys
        if (key == '\n') {
            // Enter key
            print_char('\n');

            // If command is not empty, add to history
            if (buffer_position > 0) {
                // Shift history entries down
                for (int i = CLI_HISTORY_SIZE - 1; i > 0; i--) {
                    cli_strncpy(command_history[i], command_history[i-1], CLI_MAX_COMMAND_LENGTH);
                }

                // Add current command to history
                cli_strncpy(command_history[0], command_buffer, CLI_MAX_COMMAND_LENGTH);

                if (history_count < CLI_HISTORY_SIZE) {
                    history_count++;
                }

                // Execute command
                cli_execute_command(command_buffer);
            }

            // Reset buffer and history index
            buffer_position = 0;
            for (int i = 0; i < CLI_MAX_COMMAND_LENGTH; i++) {
                command_buffer[i] = 0;
            }
            history_index = -1;

            cli_prompt();
        }
        else if (key == '\b') {
            // Backspace
            cli_handle_backspace();
        }
        else if (key == 0x1B) {
            // Escape sequence - might be an arrow key
            char second = keyboard_read();
            if (second == '[') {
                char third = keyboard_read();
                if (third == 'A') {
                    // Up arrow
                    cli_handle_up_arrow();
                }
                else if (third == 'B') {
                    // Down arrow
                    cli_handle_down_arrow();
                }
            }
        }
        else if (key >= ' ' && key <= '~' && buffer_position < CLI_MAX_COMMAND_LENGTH - 1) {
            // Printable character
            command_buffer[buffer_position++] = key;
            command_buffer[buffer_position] = 0;
            print_char(key);
        }
    }
}

int cli_execute_command(const char* command) {
    // Check for empty command
    if (command[0] == 0) {
        return 0;
    }

    // Help command
    if (cli_strcmp(command, "help") == 0) {
        print_str("Available commands:\n");
        print_str("  help     - Display this help message\n");
        print_str("  clear    - Clear the screen\n");
        print_str("  refresh  - Refresh the screen display\n");
        print_str("  echo     - Echo the arguments\n");
        print_str("  version  - Display system version\n");
        print_str("  uptime   - Show system uptime\n");
        print_str("  meow     - Make cat sounds\n");
        return 0;
    }

    // Clear screen command
    if (cli_strcmp(command, "clear") == 0) {
        print_clear();
        return 0;
    }

    if (cli_strcmp(command, "refresh") == 0) {
        print_refresh();
        print_str("Screen refreshed\n");
        return 0;
    }

    // Version command
    if (cli_strcmp(command, "version") == 0) {
        print_str("femboyOS v0.1 ALPHA\n");
        print_str("Built with love and water\n");
        return 0;
    }

    // Uptime command
    if (cli_strcmp(command, "uptime") == 0) {
        // Get tick count from timer.c
        extern volatile uint64_t tick_count;

        uint64_t seconds = tick_count / 1000;
        uint64_t minutes = seconds / 60;
        uint64_t hours = minutes / 60;

        print_str("System uptime: ");

        // Print hours
        char time_str[20];
        int pos = 0;

        if (hours > 0) {
            // Convert hours to string
            uint64_t temp = hours;
            int digit_pos = 0;
            char digits[10];

            do {
                digits[digit_pos++] = '0' + (temp % 10);
                temp /= 10;
            } while (temp > 0);

            while (digit_pos > 0) {
                time_str[pos++] = digits[--digit_pos];
            }

            time_str[pos++] = 'h';
            time_str[pos++] = ' ';
        }

        // Print minutes
        if (minutes > 0 || hours > 0) {
            uint64_t mins = minutes % 60;

            // Convert minutes to string
            int digit_pos = 0;
            char digits[10];

            do {
                digits[digit_pos++] = '0' + (mins % 10);
                mins /= 10;
            } while (mins > 0);

            while (digit_pos > 0) {
                time_str[pos++] = digits[--digit_pos];
            }

            time_str[pos++] = 'm';
            time_str[pos++] = ' ';
        }

        // Print seconds
        uint64_t secs = seconds % 60;

        // Convert seconds to string
        int digit_pos = 0;
        char digits[10];

        do {
            digits[digit_pos++] = '0' + (secs % 10);
            secs /= 10;
        } while (secs > 0);

        while (digit_pos > 0) {
            time_str[pos++] = digits[--digit_pos];
        }

        time_str[pos++] = 's';
        time_str[pos] = 0;

        print_str(time_str);
        print_str("\n");

        return 0;
    }

    // Echo command
    if (cli_strncmp(command, "echo ", 5) == 0) {
        // Create a non-const copy of the command to pass to print_str
        char echo_text[CLI_MAX_COMMAND_LENGTH];
        cli_strncpy(echo_text, command + 5, CLI_MAX_COMMAND_LENGTH);
        print_str(echo_text);
        print_str("\n");
        return 0;
    }

    // Meow command
    if (cli_strcmp(command, "meow") == 0) {
        print_str("Meow~ :3c\n");
        return 0;
    }

    // Command not found
    print_str("Unknown command: ");
    // Create a non-const copy of the command to pass to print_str
    char cmd_copy[CLI_MAX_COMMAND_LENGTH];
    cli_strncpy(cmd_copy, command, CLI_MAX_COMMAND_LENGTH);
    print_str(cmd_copy);
    print_str("\n");
    return -1;
}

static void cli_prompt() {
    // Make sure we're at the beginning of a line
    if (col > 0) {
        print_char('\n');
    }

    print_set_color(PRINT_COLOR_LIGHT_GRAY, PRINT_COLOR_BLACK);
    print_str("femboy");

    print_set_color(PRINT_COLOR_WHITE, PRINT_COLOR_BLACK);
    print_str("OS");

    print_set_color(PRINT_COLOR_LIGHT_GREEN, PRINT_COLOR_BLACK);
    print_str("> ");

    print_set_color(PRINT_COLOR_WHITE, PRINT_COLOR_BLACK);
}

static void cli_clear_line() {
    // Save current cursor position
    size_t current_col, current_row;
    print_get_cursor(&current_col, &current_row);

    // Move to beginning of line
    print_set_cursor(0, current_row);

    // Clear the entire line
    clear_row(current_row);

    // Reset cursor to beginning of line
    print_set_cursor(0, current_row);
}

static void cli_display_command() {
    cli_clear_line();
    cli_prompt();
    print_str(command_buffer);
}

static void cli_handle_backspace() {
    if (buffer_position > 0) {
        buffer_position--;
        command_buffer[buffer_position] = 0;

        // Move cursor back, print space, move back again
        print_char('\b');
        print_char(' ');
        print_char('\b');
    }
}

static void cli_handle_up_arrow() {
    if (history_count > 0 && history_index < history_count - 1) {
        history_index++;
        cli_strncpy(command_buffer, command_history[history_index], CLI_MAX_COMMAND_LENGTH);
        buffer_position = cli_strlen(command_buffer);
        cli_display_command();
    }
}

static void cli_handle_down_arrow() {
    if (history_index > 0) {
        history_index--;
        cli_strncpy(command_buffer, command_history[history_index], CLI_MAX_COMMAND_LENGTH);
        buffer_position = cli_strlen(command_buffer);
        cli_display_command();
    }
    else if (history_index == 0) {
        history_index = -1;
        buffer_position = 0;
        command_buffer[0] = 0;
        cli_display_command();
    }
}

static int cli_strcmp(const char* str1, const char* str2) {
    while (*str1 && (*str1 == *str2)) {
        str1++;
        str2++;
    }
    return *(const unsigned char*)str1 - *(const unsigned char*)str2;
}

static int cli_strncmp(const char* str1, const char* str2, int n) {
    while (n && *str1 && (*str1 == *str2)) {
        str1++;
        str2++;
        n--;
    }
    if (n == 0) {
        return 0;
    }
    return *(const unsigned char*)str1 - *(const unsigned char*)str2;
}

static void cli_strncpy(char* dest, const char* src, int n) {
    while (n > 0 && *src) {
        *dest++ = *src++;
        n--;
    }
    // Null-terminate the destination
    if (n > 0) {
        *dest = 0;
    }
}

static int cli_strlen(const char* str) {
    int len = 0;
    while (*str) {
        len++;
        str++;
    }
    return len;
}
