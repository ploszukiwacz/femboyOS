#pragma once

#include <stdint.h>
#include <stddef.h>

extern size_t col;
extern size_t row;
extern uint8_t color;

enum {
    PRINT_COLOR_BLACK = 0,
    PRINT_COLOR_BLUE = 1,
    PRINT_COLOR_GREEN = 2,
    PRINT_COLOR_CYAN = 3,
    PRINT_COLOR_RED = 4,
    PRINT_COLOR_MAGENTA = 5,
    PRINT_COLOR_BROWN = 6,
    PRINT_COLOR_LIGHT_GRAY = 7,
    PRINT_COLOR_DARK_GRAY = 8,
    PRINT_COLOR_LIGHT_BLUE = 9,
    PRINT_COLOR_LIGHT_GREEN = 10,
    PRINT_COLOR_LIGHT_CYAN = 11,
    PRINT_COLOR_LIGHT_RED = 12,
    PRINT_COLOR_PINK = 13,
    PRINT_COLOR_YELLOW = 14,
    PRINT_COLOR_WHITE = 15,
};

// Initialize the display system
void print_init();

size_t print_get_column(void);
void print_save_state(void);
void print_restore_state(void);
size_t print_get_row(void);

// Basic printing functions
void print_clear();
void print_char(char character);
void print_str(const char* string);
void print_number(uint64_t num);
void print_hex(uint8_t value);
void print_bin(uint64_t num);
void print_ip(uint32_t ip);

// Color control
void print_set_color(uint8_t foreground, uint8_t background);
// RGB color to VGA color conversion
uint8_t print_rgb_to_vga(uint32_t rgb);

// Set color using RGB values (24-bit color)
void print_set_color_rgb(uint32_t fg_rgb, uint32_t bg_rgb);

// Set color using hex string (like "#FF0000" for red)
void print_set_color_hex(const char* fg_hex, const char* bg_hex);

// Cursor control
void print_get_cursor(size_t* col, size_t* row);
void print_set_cursor(size_t col, size_t row);

// Screen control
void print_refresh();
void print_save_state(void);
void print_restore_state(void);
void update_cursor();

void print_scroll_up(size_t lines);
void print_scroll_down(size_t lines);
void print_set_scroll_offset(size_t offset);
size_t print_get_scroll_offset(void);