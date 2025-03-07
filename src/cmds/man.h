#pragma once

#include <stdint.h>

// Structure for a man page entry
typedef struct {
    const char* command;
    const char* short_desc;
    const char* usage;
    const char* long_desc;
    const char* examples;
} man_page_t;

// Function to list all available man pages
void CMD_man(const char* command);
void CMD_man_list_all();
