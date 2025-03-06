#pragma once

#include <stdint.h>
#include <stdbool.h>

// Initialize the keyboard
void keyboard_init();

// Check if a key is available to read
bool keyboard_is_key_available();

// Get the next character from the keyboard buffer (blocking)
char keyboard_read();

// Get the next character from the keyboard buffer (non-blocking)
// Returns 0 if no key is available
char keyboard_read_nonblocking();

// Wait for and return the next key press
char keyboard_wait_for_key();
