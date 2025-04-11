// #pragma once

// #include <stdbool.h>
// #include <stddef.h>

// extern cli_t cli;

// void cli_init(void);
// void cli_run(void);
// void cli_readline(char* buffer, int max_length);

#pragma once

#include <stdint.h>
#include <stdbool.h>

#define CLI_MAX_CMD_LENGTH 256
#define CLI_HISTORY_SIZE 50
#define CLI_BUFFER_SIZE 2048
typedef struct {
    char buffer[CLI_MAX_CMD_LENGTH];
    int length;
    int cursor_pos;
    bool insert_mode;
    struct {
        int count;
        int current_index;
        int buffer_pos;
        int start;
    } history;
} cli_t;

// Initialize the command line interface
void cli_init(void);

// Run the command line interface
void cli_run(void);

// Execute a command and return the result
int cli_execute_command(const char* command);

// Reset command line state after command execution
void cli_reset_after_command(void);
char* cli_strtok(char* str, const char* delim);