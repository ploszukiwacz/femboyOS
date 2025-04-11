#pragma once

#define MAX_COMMANDS 50

typedef struct {
    const char* name;          // Command name
    const char* short_desc;    // Short description of the command
    const char* usage;         // Usage information
    const char* long_desc;     // Detailed description
    const char* examples;      // Examples of usage
    void (*execute)(const char* args); // Function pointer to execute the command
} command_t;

void register_command(const command_t* cmd);
const command_t* get_registered_commands(int* count);
void initialize_command_registry(void);