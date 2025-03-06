#pragma once

#include <stdint.h>

// Initialize the command line interface
void cli_init();

// Run the command line interface (this function won't return)
void cli_run();

// Execute a command and return the result
int cli_execute_command(const char* command);
