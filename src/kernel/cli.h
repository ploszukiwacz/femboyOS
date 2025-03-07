#pragma once

#include <stdint.h>

// Initialize the command line interface
void cli_init(void);

// Run the command line interface (this function won't return)
void cli_run(void);

// Execute a command and return the result
int cli_execute_command(const char* command);

void cli_reset_after_command();
