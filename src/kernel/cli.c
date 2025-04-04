#include "cli.h"
#include "../libs/print.h"
#include "../libs/keyboard.h"
#include "../libs/timer.h"
#include "panic.h"

// Include commands
#include "../cmds/poweroff.h"
#include "../cmds/restart.h"
#include "../cmds/keytest.h"
#include "../cmds/fortune.h"
#include "../cmds/man.h"
#include "../cmds/random.h"
#include "../cmds/dance.h"
#include "../cmds/hardtest.h"

#define CLI_MAX_CMD_LENGTH 256
#define CLI_HISTORY_SIZE 50
#define CLI_BUFFER_SIZE 2048

typedef struct {
    char* commands[CLI_HISTORY_SIZE];
    int start;              // Circular buffer start
    int count;              // Number of commands in history
    int current_index;      // Current position when browsing history
    char buffer[CLI_BUFFER_SIZE];  // Storage for command strings
    int buffer_pos;         // Current position in buffer
} CommandHistory;

typedef struct {
    char buffer[CLI_MAX_CMD_LENGTH];
    int length;
    int cursor_pos;
    bool insert_mode;
    CommandHistory history;
} CommandLine;

extern struct Char {
    uint8_t character;
    uint8_t color;
} *const buffer;

static int cli_strcmp(const char* str1, const char* str2);
static int cli_strncmp(const char* str1, const char* str2, int n);
static void cli_strncpy(char* dest, const char* src, int n);
static void ensure_new_line(void);
static void redraw_line(void);
int cli_execute_command(const char* command);

static CommandLine cli;

#define NUM_ROWS 25
#define NUM_COLS 80

// static bool insert_mode = true;  // Whether we're in insert mode or overwrite mode

// External variables from print.c
extern size_t col;
extern size_t row;

// Forward declarations
static int cli_strcmp(const char* str1, const char* str2);
static int cli_strncmp(const char* str1, const char* str2, int n);
static void cli_strncpy(char* dest, const char* src, int n);

void cli_init() {
    // Disable interrupts during initialization
    // asm volatile("cli");

    // Clear command buffer
    for (int i = 0; i < CLI_MAX_CMD_LENGTH; i++) {
        cli.buffer[i] = 0;
    }

    // Initialize CLI state
    cli.length = 0;
    cli.cursor_pos = 0;
    cli.insert_mode = true;
    cli.history.count = 0;
    cli.history.current_index = -1;
    cli.history.buffer_pos = 0;
    cli.history.start = 0;

    // Clear screen and set initial colors
    print_clear();
    print_set_color(PRINT_COLOR_LIGHT_CYAN, PRINT_COLOR_BLACK);
    print_str("************************************\n");
    print_str("*       FemboyOS v0.1 ALPHA        *\n");
    print_str("*          AGPL Licensed           *\n");
    print_str("* github.com/ploszukiwacz/femboyOS *\n");
    print_str("************************************\n\n");
    print_set_color(PRINT_COLOR_LIGHT_GRAY, PRINT_COLOR_BLACK);
    print_str("Type 'help' for available commands.\n\n");

    // Re-enable interrupts
    // asm volatile("sti");

    // Make sure cursor is visible
    update_cursor();
}
// Add command to history
static void add_to_history(const char* cmd) {
    if (cli.length == 0) return;  // Don't add empty commands

    // Calculate required space
    int len = cli.length + 1;  // Include null terminator

    // Check if we have enough space in buffer
    if (cli.history.buffer_pos + len > CLI_BUFFER_SIZE) {
        // Reset buffer if full
        cli.history.buffer_pos = 0;
    }

    // Copy command to buffer
    char* cmd_copy = &cli.history.buffer[cli.history.buffer_pos];
    for (int i = 0; i < len; i++) {
        cmd_copy[i] = cmd[i];
    }
    cli.history.buffer_pos += len;

    // Add to history
    if (cli.history.count < CLI_HISTORY_SIZE) {
        // History not full yet
        cli.history.commands[cli.history.count++] = cmd_copy;
    } else {
        // History full, remove oldest command
        cli.history.start = (cli.history.start + 1) % CLI_HISTORY_SIZE;
        cli.history.commands[(cli.history.start + cli.history.count - 1) % CLI_HISTORY_SIZE] = cmd_copy;
    }
}

static void redraw_line(void) {
    size_t current_col, current_row;
    print_get_cursor(&current_col, &current_row);

    // Clear current line
    print_set_cursor(0, current_row);
    for (size_t i = 0; i < NUM_COLS; i++) {
        print_char(' ');
    }

    // Move back to start and draw prompt
    print_set_cursor(0, current_row);

    // Draw prompt
    print_set_color(PRINT_COLOR_LIGHT_GRAY, PRINT_COLOR_BLACK);
    print_str("femboy");
    print_set_color(PRINT_COLOR_WHITE, PRINT_COLOR_BLACK);
    print_str("OS");
    print_set_color(PRINT_COLOR_LIGHT_GREEN, PRINT_COLOR_BLACK);
    print_str("> ");

    // Print command
    print_set_color(PRINT_COLOR_WHITE, PRINT_COLOR_BLACK);
    print_str(cli.buffer);

    // Position cursor
    print_set_cursor(cli.cursor_pos + 9, current_row);
}
// Load command from history
static void load_history_command(int index) {
    if (index < 0 || index >= cli.history.count) return;

    // Copy command to buffer
    int i;
    for (i = 0; cli.history.commands[index][i] && i < CLI_MAX_CMD_LENGTH - 1; i++) {
        cli.buffer[i] = cli.history.commands[index][i];
    }
    cli.buffer[i] = 0;
    cli.length = i;
    cli.cursor_pos = i;

    redraw_line();
}

// Handle character insertion
static void insert_char(char c) {
    if (cli.length >= CLI_MAX_CMD_LENGTH - 1) return;

    if (cli.insert_mode || cli.cursor_pos == cli.length) {
        // Make space for new character
        for (int i = cli.length; i > cli.cursor_pos; i--) {
            cli.buffer[i] = cli.buffer[i - 1];
        }
        cli.buffer[cli.cursor_pos] = c;
        cli.length++;
        cli.cursor_pos++;
    } else {
        // Overwrite mode
        cli.buffer[cli.cursor_pos] = c;
        cli.cursor_pos++;
        if (cli.cursor_pos > cli.length) {
            cli.length = cli.cursor_pos;
        }
    }

    cli.buffer[cli.length] = 0;
    redraw_line();
}

// Handle backspace
static void handle_backspace() {
    if (cli.cursor_pos > 0) {
        for (int i = cli.cursor_pos - 1; i < cli.length - 1; i++) {
            cli.buffer[i] = cli.buffer[i + 1];
        }
        cli.cursor_pos--;
        cli.length--;
        cli.buffer[cli.length] = 0;
        redraw_line();
    }
}



void cli_run() {
    redraw_line();  // Initial prompt

    while (1) {
        // Wait for key input with interrupts enabled
        char c = keyboard_read();  // This should block until a key is pressed
        // insert_char(c);

        // Process the key
        if (c == KEY_ARROW_UP) {
            if (cli.history.current_index < cli.history.count - 1) {
                cli.history.current_index++;
                load_history_command(cli.history.current_index);
            }
        }
        else if (c == KEY_ARROW_DOWN) {
            if (cli.history.current_index > 0) {
                cli.history.current_index--;
                load_history_command(cli.history.current_index);
            } else if (cli.history.current_index == 0) {
                cli.history.current_index = -1;
                cli.length = 0;
                cli.cursor_pos = 0;
                cli.buffer[0] = 0;
                redraw_line();
            }
        }
        else if (c == KEY_ARROW_LEFT) {
            if (cli.cursor_pos > 0) {
                cli.cursor_pos--;
                redraw_line();
            }
        }
        else if (c == KEY_ARROW_RIGHT) {
            if (cli.cursor_pos < cli.length) {
                cli.cursor_pos++;
                redraw_line();
            }
        }
        else if (c == '\n') {
            print_char('\n');
            if (cli.length > 0) {
                // Save command to history
                add_to_history(cli.buffer);

                // Execute command
                cli_execute_command(cli.buffer);

                // Reset command line
                cli.length = 0;
                cli.cursor_pos = 0;
                cli.buffer[0] = 0;
                cli.history.current_index = -1;
            }
            redraw_line();
        }
        else if (c == '\b') {
            handle_backspace();
        }
        else if (c >= ' ' && c <= '~') {  // Printable characters
            insert_char(c);
        }

        // Update screen and cursor after every key press
        update_cursor();
    }
}

int cli_execute_command(const char* command) {
    print_save_state();

    // Check for empty command
    if (command[0] == 0) {
        return 0;
    }

    // Help command
    if (cli_strcmp(command, "help") == 0) {
        print_str("Available commands:\n");
        print_str("  help     - Display this help message\n");
        print_str("  clear    - Clear the screen\n");
        print_str("  calc     - Calc (+ - / *)\n");
        print_str("  refresh  - Refresh the screen display\n");
        print_str("  echo     - Echo the arguments\n");
        print_str("  version  - Display system version\n");
        print_str("  uptime   - Show system uptime\n");
        print_str("  sysfetch - Display system info\n");
        print_str("  man      - Display system manual pages\n");
        print_str("  tail     - Display the last lines of text\n");
        print_str("  less     - View text with paging\n");
        print_str("  dance    - Little dancing character\n");
        print_str("  fortune  - Random fortune message\n");
        print_str("  random   - Give a random number from rang\n");
        print_str("  panic    - Trigger a kernel panic\n");
        print_str("  poweroff - Poweroff the pc\n");
        print_str("  restart  - Restart the pc\n");
        return 0;
    }

    // Clear screen command
    if (cli_strcmp(command, "clear") == 0) {
        print_clear();
        return 0;
    }

    if (cli_strncmp(command, "calc ", 5) == 0) {
        const char* expression = command + 5;

        // Variables to store the calculation
        int num1 = 0;
        int num2 = 0;
        char op = 0;
        int result = 0;

        // Simple state machine to parse "num1 op num2"
        int state = 0;  // 0: parsing num1, 1: parsing op, 2: parsing num2

        for (int i = 0; expression[i] != 0; i++) {
            char c = expression[i];

            // Skip whitespace
            if (c == ' ') {
                if (state == 0 && num1 != 0) state = 1;  // Transition to op state if num1 is parsed
                continue;
            }

            if (state == 0) {
                // Parsing num1
                if (c >= '0' && c <= '9') {
                    num1 = num1 * 10 + (c - '0');
                } else {
                    // If we encounter a non-number while parsing num1, it must be the operator
                    op = c;
                    state = 2;  // Move to parsing num2
                }
            } else if (state == 1) {
                // Parsing op
                if (c == '+' || c == '-' || c == '*' || c == '/') {
                    op = c;
                    state = 2;  // Move to parsing num2
                }
            } else if (state == 2) {
                // Parsing num2
                if (c >= '0' && c <= '9') {
                    num2 = num2 * 10 + (c - '0');
                }
            }
        }

        // Perform the calculation
        if (op == '+') {
            result = num1 + num2;
        } else if (op == '-') {
            result = num1 - num2;
        } else if (op == '*') {
            result = num1 * num2;
        } else if (op == '/') {
            if (num2 == 0) {
                print_str("Error: Division by zero\n");
                return 0;
            }
            result = num1 / num2;
        } else {
            print_str("Error: Invalid operator. Use +, -, *, or /\n");
            return 0;
        }

        // Print the result
        print_str("Result: ");

        // Convert result to string and print
        if (result < 0) {
            print_char('-');
            result = -result;
        }

        print_number(result);
        print_str("\n");

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
        char echo_text[CLI_MAX_CMD_LENGTH];
        cli_strncpy(echo_text, command + 5, CLI_MAX_CMD_LENGTH);
        print_str(echo_text);
        print_str("\n");
        return 0;
    }

    // Neofetch command
    if (cli_strcmp(command, "sysfetch") == 0) {
        // Save current color setup
        uint8_t old_fg = PRINT_COLOR_WHITE;
        uint8_t old_bg = PRINT_COLOR_BLACK;

        print_set_color(PRINT_COLOR_WHITE, PRINT_COLOR_BLACK);
        print_str("\n");

        // OS
        print_set_color(PRINT_COLOR_LIGHT_GRAY, PRINT_COLOR_BLACK);
        print_str("OS: ");
        print_set_color(PRINT_COLOR_WHITE, PRINT_COLOR_BLACK);
        print_str("femboyOS v0.1 ALPHA\n");

        // Kernel
        print_set_color(PRINT_COLOR_LIGHT_GRAY, PRINT_COLOR_BLACK);
        print_str("Kernel: ");
        print_set_color(PRINT_COLOR_WHITE, PRINT_COLOR_BLACK);
        print_str("0.1 x86_64\n");

        print_set_color(PRINT_COLOR_LIGHT_GRAY, PRINT_COLOR_BLACK);
        print_str("Uptime: ");

        // Get uptime
        extern volatile uint64_t tick_count;

        uint64_t seconds = tick_count / 1000;
        uint64_t minutes = seconds / 60;
        uint64_t hours = minutes / 60;

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

        print_set_color(PRINT_COLOR_WHITE, PRINT_COLOR_BLACK);
        print_str(time_str);
        print_str("\n");

        // Get CPU info
        char cpu_info[64] = "Unknown CPU";
        uint32_t cpu_signature = 0;
        uint32_t cpu_features_edx = 0;
        uint32_t cpu_features_ecx = 0;

        // Use CPUID instruction to get CPU info
        __asm__ volatile (
            "cpuid"
            : "=a" (cpu_signature), "=c" (cpu_features_ecx), "=d" (cpu_features_edx)
            : "a" (1)
            : "ebx"
        );

        // Extract CPU info
        uint8_t cpu_model = (cpu_signature >> 4) & 0xF;
        uint8_t cpu_family = (cpu_signature >> 8) & 0xF;
        uint8_t cpu_ext_model = (cpu_signature >> 16) & 0xF;
        uint8_t cpu_ext_family = (cpu_signature >> 20) & 0xFF;

        // Format CPU model string
        if (cpu_family == 0xF) {
            cpu_family += cpu_ext_family;
            cpu_model += (cpu_ext_model << 4);
        }

        // Create simple CPU info string
        // In a real OS, we'd use more CPUID calls to get the actual brand string
        cli_strncpy(cpu_info, "x86_64 CPU", sizeof(cpu_info));

        print_set_color(PRINT_COLOR_LIGHT_GRAY, PRINT_COLOR_BLACK);
        print_str("CPU: ");
        print_set_color(PRINT_COLOR_WHITE, PRINT_COLOR_BLACK);
        print_str(cpu_info);

        // Display CPU details if available
        if (cpu_signature != 0) {
            print_str(" Family ");
            char num_str[8];
            int pos = 0;
            uint32_t temp = cpu_family;
            do {
                num_str[pos++] = '0' + (temp % 10);
                temp /= 10;
            } while (temp > 0);
            num_str[pos] = '\0';
            // Reverse the string
            for (int i = 0; i < pos / 2; i++) {
                char c = num_str[i];
                num_str[i] = num_str[pos - 1 - i];
                num_str[pos - 1 - i] = c;
            }
            print_str(num_str);

            print_str(" Model ");
            pos = 0;
            temp = cpu_model;
            do {
                num_str[pos++] = '0' + (temp % 10);
                temp /= 10;
            } while (temp > 0);
            num_str[pos] = '\0';
            // Reverse the string
            for (int i = 0; i < pos / 2; i++) {
                char c = num_str[i];
                num_str[i] = num_str[pos - 1 - i];
                num_str[pos - 1 - i] = c;
            }
            print_str(num_str);
        }
        print_str("\n");


        // print_str("CPU: VirtualCPU @ 1.xx GHz\n");
        // print_str("Memory: 640K (that should be enough)\n");

        // Restore color
        print_set_color(old_fg, old_bg);

        return 0;
    }

    // Man command
    if (cli_strncmp(command, "man ", 4) == 0) {
        CMD_man(command);
        return 0;
    }
    if (cli_strcmp(command, "man") == 0) {
        CMD_man_list_all();
        return 0;
    }

    // Tail command
    if (cli_strncmp(command, "tail ", 5) == 0) {
        const char* text = command + 5;

        // Count the number of lines
        int total_lines = 1; // Start with 1 for the last line that might not end with \n
        for (int i = 0; text[i] != 0; i++) {
            if (text[i] == '\n') {
                total_lines++;
            }
        }

        // Determine how many lines to display (last 10 or all if less than 10)
        int lines_to_display = (total_lines < 10) ? total_lines : 10;
        int start_line = total_lines - lines_to_display;

        // Find the position to start displaying
        int current_line = 0;
        int start_pos = 0;

        for (int i = 0; text[i] != 0; i++) {
            if (text[i] == '\n') {
                current_line++;
                if (current_line == start_line) {
                    start_pos = i + 1;
                    break;
                }
            }
        }

        // Display the last 'lines_to_display' lines
        if (start_line > 0) {
            print_str("--- Last ");
            print_number(lines_to_display);
            print_str(" lines ---\n");
            print_str(text + start_pos);
        } else {
            // If the text has fewer than 10 lines, just print it all
            print_str(text);
        }

        return 0;
    }
    if (cli_strcmp(command, "tail") == 0) {
        print_str("Usage: tail [text]\n");
        print_str("Displays the last 10 lines of text.\n");
        return 0;
    }

    // Less command
    if (cli_strncmp(command, "less ", 5) == 0) {
        const char* text = command + 5;
        int text_length = 0;

        // Get text length
        while (text[text_length] != 0) {
            text_length++;
        }

        if (text_length == 0) {
            print_str("No text to display.\n");
            return 0;
        }

        // Variables for paging
        int current_pos = 0;
        int lines_per_page = NUM_ROWS - 2; // Leave room for status line
        bool exit_viewer = false;

        // Clear screen to start fresh
        print_clear();

        while (!exit_viewer) {
            // Clear screen for new page
            print_clear();

            // Display one page of text
            int current_line = 0;
            int i = current_pos;

            while (text[i] != 0 && current_line < lines_per_page) {
                print_char(text[i]);

                if (text[i] == '\n') {
                    current_line++;
                }

                i++;
            }

            // Calculate approx percentage
            int percentage = (current_pos * 100) / text_length;
            if (percentage > 100) percentage = 100;

            // Display status line
            print_set_color(PRINT_COLOR_BLACK, PRINT_COLOR_WHITE);
            print_set_cursor(0, NUM_ROWS - 1);

            // Clear status line
            for (int j = 0; j < NUM_COLS; j++) {
                print_char(' ');
            }

            print_set_cursor(0, NUM_ROWS - 1);
            print_str("Use arrows to navigate, q to quit. ");

            // Show percentage
            print_str("[");
            print_number(percentage);
            print_str("%]");

            // Reset colors
            print_set_color(PRINT_COLOR_WHITE, PRINT_COLOR_BLACK);

            // Wait for key press
            char key = keyboard_read();

            if (key == 'q' || key == 'Q') {
                // Quit
                exit_viewer = true;
            } else if (key == 0x1B) {
                // Arrow keys (escape sequence)
                char second = keyboard_read();
                if (second == '[') {
                    char third = keyboard_read();

                    if (third == 'A') {
                        // Up arrow - move up a few lines
                        int lines_to_move = 3;
                        int new_pos = current_pos;

                        // Find position that's a few lines up
                        while (lines_to_move > 0 && new_pos > 0) {
                            new_pos--;
                            if (new_pos > 0 && text[new_pos - 1] == '\n') {
                                lines_to_move--;
                            }
                        }

                        // Find the start of the line
                        while (new_pos > 0 && text[new_pos - 1] != '\n') {
                            new_pos--;
                        }

                        current_pos = new_pos;
                    } else if (third == 'B') {
                        // Down arrow - move down a line
                        if (text[current_pos] != 0) {
                            // Find the next line
                            while (text[current_pos] != 0 && text[current_pos] != '\n') {
                                current_pos++;
                            }

                            // Move past the newline character
                            if (text[current_pos] == '\n') {
                                current_pos++;
                            }
                        }
                    } else if (third == 'C') {
                        // Right arrow - next page
                        int i = current_pos;
                        int lines = 0;

                        while (text[i] != 0 && lines < lines_per_page) {
                            if (text[i] == '\n') {
                                lines++;
                            }
                            i++;
                        }

                        current_pos = i;
                    } else if (third == 'D') {
                        // Left arrow - previous page
                        int lines_to_move = lines_per_page * 2; // Move back two pages worth
                        int new_pos = current_pos;

                        while (lines_to_move > 0 && new_pos > 0) {
                            new_pos--;
                            if (new_pos > 0 && text[new_pos - 1] == '\n') {
                                lines_to_move--;
                            }
                        }

                        // Find the start of the line
                        while (new_pos > 0 && text[new_pos - 1] != '\n') {
                            new_pos--;
                        }

                        current_pos = new_pos;
                    }
                }
            } else if (key == ' ') {
                // Space - next page (same as right arrow)
                int i = current_pos;
                int lines = 0;

                while (text[i] != 0 && lines < lines_per_page) {
                    if (text[i] == '\n') {
                        lines++;
                    }
                    i++;
                }

                current_pos = i;
            }
        }

        // Clear screen when exiting
        print_clear();
        return 0;
    }
    if (cli_strcmp(command, "less") == 0) {
        print_str("Usage: less [text]\n");
        print_str("View text with paging functionality.\n");
        return 0;
    }

    // Dance command
    if (cli_strcmp(command, "dance") == 0) {
        CMD_dance();
        return 0;
    }

    // Fortune command
    if (cli_strcmp(command, "fortune") == 0) {
        CMD_fortune();
        return 0;
    }

    // Keytest command
    if (cli_strcmp(command, "keytest") == 0) {
        CMD_keytest();
        return 0;
    }

    if (cli_strcmp(command, "panic") == 0) {
        PANIC("Manual panic trigger.");
        return 0;
    }

    if (cli_strcmp(command, "hardtest") == 0) {
        CMD_hardtest();
        return 0;
    }

    // Random command
    if (cli_strncmp(command, "random", 6) == 0 && (command[6] == '\0' || command[6] == ' ')) {
        return CMD_random(command);
    }

    // Asm Command
    if (cli_strncmp(command, "asm ", 4) == 0) {
        const char* bytes_str = command + 4;
            uint8_t* code = (uint8_t*)0x70000; // Fixed executable memory region
            int code_len = 0;

            // Parse space-separated hex bytes (like "90 F4 C3" for nop; hlt; ret)
            while (*bytes_str) {
                // Skip spaces
                while (*bytes_str == ' ') bytes_str++;
                if (!*bytes_str) break;

                // Parse two hex digits
                uint8_t byte = 0;
                for (int i = 0; i < 2; i++) {
                    char c = *bytes_str++;
                    byte <<= 4;
                    if (c >= '0' && c <= '9') byte |= c - '0';
                    else if (c >= 'a' && c <= 'f') byte |= c - 'a' + 10;
                    else if (c >= 'A' && c <= 'F') byte |= c - 'A' + 10;
                    else {
                        print_str("Invalid hex digit\n");
                        return -1;
                    }
                }

                code[code_len++] = byte;
            }

            // Add ret instruction if not already present
            if (code[code_len-1] != 0xC3) {
                code[code_len++] = 0xC3;
            }

            print_str("Executing code...\n");
            ((void(*)())code)();
            print_str("Done!\n");
            return 0;
    }

    // Poweroff command
    if (cli_strcmp(command, "poweroff") == 0 || cli_strcmp(command, "shutdown") == 0) {
        CMD_poweroff();
        return 0;
    }

    // Reboot command
    if (cli_strcmp(command, "reboot") == 0 || cli_strcmp(command, "restart") == 0) {
        CMD_restart();
        return 0;
    }

    // Command not found
    print_str("Unknown command: ");
    // Create a non-const copy of the command to pass to print_str
    char cmd_copy[CLI_MAX_CMD_LENGTH];
    cli_strncpy(cmd_copy, command, CLI_MAX_CMD_LENGTH);
    print_str(cmd_copy);
    print_str("\n");

    print_restore_state();

        // Ensure we end with a newline
        if (print_get_column() > 0) {
            print_char('\n');
        }

    return -1;
}

int cli_strcmp(const char* str1, const char* str2) {
    while (*str1 && (*str1 == *str2)) {
        str1++;
        str2++;
    }
    return *(const unsigned char*)str1 - *(const unsigned char*)str2;
}

int cli_strncmp(const char* str1, const char* str2, int n) {
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

void cli_strncpy(char* dest, const char* src, int n) {
    while (n > 0 && *src) {
        *dest++ = *src++;
        n--;
    }
    // Null-terminate the destination
    if (n > 0) {
        *dest = 0;
    }
}

void cli_reset_after_command() {
    // Reset buffer
    cli.length = 0;
    cli.cursor_pos = 0;
    cli.buffer[0] = 0;
    cli.history.current_index = -1;

    // Ensure we're at a new line
    ensure_new_line();

    // Redraw the prompt
    redraw_line();
}

void ensure_new_line(void) {
    size_t current_col = print_get_column();
    if (current_col > 0) {
        print_char('\n');
    }
}
