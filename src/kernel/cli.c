// #include "cli.h"
// #include "../libs/print.h"
// #include "../libs/string.h"
// #include "../libs/keyboard.h"

// cli_t cli;

// void cli_init() {
//     // Disable interrupts during initialization
//     // asm volatile("cli");

//     // Clear command buffer
//     for (int i = 0; i < CLI_MAX_CMD_LENGTH; i++) {
//         cli.buffer[i] = 0;
//     }

//     // Initialize CLI state
//     cli.length = 0;
//     cli.cursor_pos = 0;
//     cli.insert_mode = true;
//     cli.history.count = 0;
//     cli.history.current_index = -1;
//     cli.history.buffer_pos = 0;
//     cli.history.start = 0;

//     // Clear screen and set initial colors
//     print_clear();
//     print_set_color(PRINT_COLOR_LIGHT_CYAN, PRINT_COLOR_BLACK);
//     print_str("************************************\n");
//     print_str("*       FemboyOS v0.1 ALPHA        *\n");
//     print_str("*          AGPL Licensed           *\n");
//     print_str("* github.com/ploszukiwacz/femboyOS *\n");
//     print_str("************************************\n\n");
//     print_set_color(PRINT_COLOR_LIGHT_GRAY, PRINT_COLOR_BLACK);
//     print_str("Type 'help' for available commands.\n\n");

//     // Re-enable interrupts
//     // asm volatile("sti");

//     // Make sure cursor is visible
//     update_cursor();
// }

// void cli_run() {
//     char input_buffer[256];

//     while (1) {
//         print_str("> ");
//         cli_readline(input_buffer, sizeof(input_buffer)); // Function to read user input

//         // Parse the command name and arguments
//         char* command_name = cli_strtok(input_buffer, " ");
//         char* command_args = cli_strtok(NULL, "\0");

//         if (command_name == NULL) {
//             continue;
//         }

//         // Search for the command in the registry
//         int command_count;
//         const command_t* commands = get_registered_commands(&command_count);
//         int found = 0;

//         for (int i = 0; i < command_count; i++) {
//             if (cli_strcmp(commands[i].name, command_name) == 0) {
//                 commands[i].execute(command_args ? command_args : "");
//                 found = 1;
//                 break;
//             }
//         }

//         if (!found) {
//             print_str("Unknown command: ");
//             print_str(command_name);
//             print_str("\n");
//         }
//     }
// }

// void cli_readline(char* buffer, int max_length) {
//     int index = 0;
//     char c;

//     while (index < max_length - 1) {
//         c = keyboard_get_char(); // Function to get a character from the keyboard
//         if (c == '\n') {
//             break;
//         } else if (c == '\b' && index > 0) {
//             index--;
//             print_str("\b \b"); // Handle backspace
//         } else if (c >= 32 && c <= 126) { // Printable characters
//             buffer[index++] = c;
//             print_char(c);
//         }
//     }

//     buffer[index] = '\0';
//     print_str("\n");
// }

// int cli_strcmp(const char* str1, const char* str2) {
//     while (*str1 && (*str1 == *str2)) {
//         str1++;
//         str2++;
//     }
//     return *(unsigned char*)str1 - *(unsigned char*)str2;
// }

#include "cli.h"
#include "../libs/print.h"
#include "../libs/keyboard.h"
#include "../libs/timer.h"
#include "panic.h"
#include "../cmds/command_registry.h"

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
        if (c == KEY_ARROW_DOWN) {
            if (cli.history.current_index == -1 && cli.history.count > 0) {
                cli.history.current_index = cli.history.count - 1;
            } else if (cli.history.current_index < cli.history.count - 1) {
                cli.history.current_index++;
            }
            load_history_command(cli.history.current_index);
        }
        else if (c == KEY_ARROW_UP) {
            if (cli.history.current_index == -1 && cli.history.count > 0) {
                cli.history.current_index = cli.history.count - 1;
            } else if (cli.history.current_index > 0) {
                cli.history.current_index--;
            }
            load_history_command(cli.history.current_index);
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
    char* command_name = cli_strtok(command, " ");
    char* command_args = cli_strtok(NULL, "\0");

    if (command_name == NULL) {
        return 0; // No command to execute
    }

    int command_count;
    const command_t* commands = get_registered_commands(&command_count);

    for (int i = 0; i < command_count; i++) {
        if (cli_strcmp(commands[i].name, command_name) == 0) {
            commands[i].execute(command_args ? command_args : "");
            return 1; // Command executed successfully
        }
    }

    print_str("Unknown command: ");
    print_str(command_name);
    print_str("\n");
    return 0; // Command not found
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

char* cli_strtok(char* str, const char* delim) {
    static char* next_token = NULL;

    if (str != NULL) {
        next_token = str;
    }

    if (next_token == NULL) {
        return NULL;
    }

    char* token_start = next_token;

    while (*next_token != '\0') {
        for (const char* d = delim; *d != '\0'; d++) {
            if (*next_token == *d) {
                *next_token = '\0';
                next_token++;
                return token_start;
            }
        }
        next_token++;
    }

    next_token = NULL;
    return token_start;
}