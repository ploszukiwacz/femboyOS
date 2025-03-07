#pragma once

#include <stdint.h>
#include <stdbool.h>

// Define Keys
#define KEY_HOME    0x01  // Special code to represent Home key
#define KEY_END     0x02  // Special code to represent End key
#define KEY_DELETE  0x03  // Special code to represent Delete key
#define KEY_INSERT  0x04  // Special code to represent Insert key
#define KEY_PGUP    0x05  // Special code to represent Page Up key
#define KEY_PGDN    0x06  // Special code to represent Page Down key
#define KEY_ARROW_UP    0x10  // Special code to represent Up arrow
#define KEY_ARROW_DOWN  0x11  // Special code to represent Down arrow
#define KEY_ARROW_LEFT  0x12  // Special code to represent Left arrow
#define KEY_ARROW_RIGHT 0x13  // Special code to represent Right arrow

#define SCROLL_BUFFER_ROWS 1000

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
