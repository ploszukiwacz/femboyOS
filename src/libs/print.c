#include "print.h"
#include "port.h"
#include "keyboard.h"
#include "string.h"

static const size_t NUM_COLS = 80;
static const size_t NUM_ROWS = 25;

// VGA control registers
#define VGA_CTRL_REGISTER 0x3D4
#define VGA_DATA_REGISTER 0x3D5
#define VGA_CURSOR_HIGH 0x0E
#define VGA_CURSOR_LOW 0x0F

struct Char {
    uint8_t character;
    uint8_t color;
};

struct Char* const buffer = (struct Char*) 0xb8000;
size_t col = 0;
size_t row = 0;
uint8_t color = PRINT_COLOR_WHITE | (PRINT_COLOR_BLACK << 4);
// Add these implementations
size_t print_get_column(void) {
    return col;
}
static size_t scroll_offset = 0;
static struct Char scroll_buffer[1000][80];
static bool autoscroll = true;
static size_t buffer_used_rows = 0;  // Track how many rows we've actually used
static size_t total_rows = 0;

size_t print_get_row(void) {
    return row;
}

// Function to update the hardware cursor
void update_cursor() {
    uint16_t pos = row * NUM_COLS + col;

    port_byte_out(0x3D4, 0x0F);
    port_byte_out(0x3D5, (uint8_t)(pos & 0xFF));
    port_byte_out(0x3D4, 0x0E);
    port_byte_out(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

// Function to enable the cursor
void enable_cursor() {
    port_byte_out(VGA_CTRL_REGISTER, 0x0A);
    port_byte_out(VGA_DATA_REGISTER, (port_byte_in(VGA_DATA_REGISTER) & 0xC0) | 0x0E);
    port_byte_out(VGA_CTRL_REGISTER, 0x0B);
    port_byte_out(VGA_DATA_REGISTER, (port_byte_in(VGA_DATA_REGISTER) & 0xE0) | 0x0F);
}

static void copy_line_to_buffer(size_t screen_row, size_t buffer_row) {
    for (size_t c = 0; c < NUM_COLS; c++) {
        scroll_buffer[buffer_row][c] = buffer[c + NUM_COLS * screen_row];
    }
}

static void copy_line_from_buffer(size_t buffer_row, size_t screen_row) {
    for (size_t c = 0; c < NUM_COLS; c++) {
        buffer[c + NUM_COLS * screen_row] = scroll_buffer[buffer_row][c];
    }
}

// Modified scroll() function to properly maintain buffer
static void scroll() {
    // First, save current screen to buffer
    for (size_t r = 0; r < NUM_ROWS; r++) {
        for (size_t c = 0; c < NUM_COLS; c++) {
            scroll_buffer[buffer_used_rows + r][c] = buffer[r * NUM_COLS + c];
        }
    }

    // Update buffer usage
    buffer_used_rows += NUM_ROWS;
    if (buffer_used_rows >= SCROLL_BUFFER_ROWS) {
        // If buffer is full, shift everything up
        for (size_t r = 0; r < SCROLL_BUFFER_ROWS - NUM_ROWS; r++) {
            for (size_t c = 0; c < NUM_COLS; c++) {
                scroll_buffer[r][c] = scroll_buffer[r + NUM_ROWS][c];
            }
        }
        buffer_used_rows = SCROLL_BUFFER_ROWS - NUM_ROWS;
    }

    // Normal screen scroll
    for (size_t r = 1; r < NUM_ROWS; r++) {
        for (size_t c = 0; c < NUM_COLS; c++) {
            buffer[c + NUM_COLS * (r-1)] = buffer[c + NUM_COLS * r];
        }
    }
    clear_row(NUM_ROWS - 1);
}

static void clear_row(size_t row) {
    struct Char empty = (struct Char) {
        .character = ' ',
        .color = color,
    };

    for (size_t c = 0; c < NUM_COLS; c++) {
        buffer[c + NUM_COLS * row] = empty;
    }
}

static void clear_buffer_row(size_t row) {
    struct Char empty = (struct Char) {
        .character = ' ',
        .color = color,
    };
    for (size_t c = 0; c < NUM_COLS; c++) {
        scroll_buffer[row][c] = empty;
    }
}

static void scroll_buffer_up(void) {
    // Move everything up one line
    memmove(&scroll_buffer[0], &scroll_buffer[1],
            sizeof(struct Char) * NUM_COLS * (SCROLL_BUFFER_ROWS - 1));
    clear_buffer_row(SCROLL_BUFFER_ROWS - 1);
}

void print_scroll_up(size_t lines) {
    if (buffer_used_rows > NUM_ROWS) {
        if (scroll_offset + lines > buffer_used_rows - NUM_ROWS) {
            scroll_offset = buffer_used_rows - NUM_ROWS;
        } else {
            scroll_offset += lines;
        }

        // Update screen from buffer
        for (size_t r = 0; r < NUM_ROWS; r++) {
            for (size_t c = 0; c < NUM_COLS; c++) {
                buffer[r * NUM_COLS + c] = scroll_buffer[scroll_offset + r][c];
            }
        }
        update_cursor();
    }
}

void print_scroll_down(size_t lines) {
    if (scroll_offset > 0) {
        if (lines > scroll_offset) {
            scroll_offset = 0;
        } else {
            scroll_offset -= lines;
        }

        // Update screen from buffer
        for (size_t r = 0; r < NUM_ROWS; r++) {
            for (size_t c = 0; c < NUM_COLS; c++) {
                buffer[r * NUM_COLS + c] = scroll_buffer[scroll_offset + r][c];
            }
        }
        update_cursor();
    }
}

static void update_screen() {
    for (size_t r = 0; r < NUM_ROWS; r++) {
        size_t buffer_row = r + scroll_offset;
        if (buffer_row < SCROLL_BUFFER_ROWS) {
            for (size_t c = 0; c < NUM_COLS; c++) {
                buffer[c + NUM_COLS * r] = scroll_buffer[buffer_row][c];
            }
        }
    }
    update_cursor();
}
void print_clear() {
    struct Char empty = (struct Char) {
        .character = ' ',
        .color = color,
    };

    // Clear scroll buffer
    for (size_t r = 0; r < SCROLL_BUFFER_ROWS; r++) {
        for (size_t c = 0; c < NUM_COLS; c++) {
            scroll_buffer[r][c] = empty;
        }
    }

    // Clear screen
    for (size_t r = 0; r < NUM_ROWS; r++) {
        clear_row(r);
    }

    scroll_offset = 0;
    buffer_used_rows = 0;
    col = 0;
    row = 0;
    update_cursor();
}


void print_init() {
    // Initialize scroll buffer
    struct Char empty = (struct Char) {
        .character = ' ',
        .color = PRINT_COLOR_WHITE | (PRINT_COLOR_BLACK << 4)
    };

    for (size_t r = 0; r < SCROLL_BUFFER_ROWS; r++) {
        for (size_t c = 0; c < NUM_COLS; c++) {
            scroll_buffer[r][c] = empty;
        }
    }

    // Clear screen
    print_clear();
    enable_cursor();
    update_cursor();
}

void print_newline() {
    col = 0;

    if (row < NUM_ROWS - 1) {
        row++;
    } else {
        scroll();
    }

    update_cursor();
}

void print_char(char character) {
    switch (character) {
        case '\n':
            col = 0;
            if (row < NUM_ROWS - 1) {
                row++;
            } else {
                scroll();
            }
            break;

        default:
            if (col >= NUM_COLS) {
                col = 0;
                if (row < NUM_ROWS - 1) {
                    row++;
                } else {
                    scroll();
                }
            }

            // Write to screen
            buffer[col + NUM_COLS * row] = (struct Char) {
                .character = (uint8_t)character,
                .color = color,
            };

            // Write to scroll buffer
            size_t buffer_row = buffer_used_rows + row;
            if (buffer_row < SCROLL_BUFFER_ROWS) {
                scroll_buffer[buffer_row][col] = buffer[col + NUM_COLS * row];
            }

            col++;
    }
    update_cursor();
}

void print_str(const char* str) {
    for (size_t i = 0; str[i] != '\0'; i++) {
        print_char(str[i]);
    }
}

void print_set_color(uint8_t foreground, uint8_t background) {
    // Ensure values are in valid range
    foreground &= 0x0F;
    background &= 0x0F;
    color = foreground | (background << 4);
}

void print_get_cursor(size_t* out_col, size_t* out_row) {
    if (out_col) *out_col = col;
    if (out_row) *out_row = row;
}

void print_set_cursor(size_t new_col, size_t new_row) {
    if (new_col < NUM_COLS && new_row < NUM_ROWS) {
        col = new_col;
        row = new_row;
        update_cursor();
    }
}

void print_refresh() {
    // Copy visible portion of scroll buffer to screen
    for (size_t r = 0; r < NUM_ROWS; r++) {
        size_t buffer_row = r + scroll_offset;
        for (size_t c = 0; c < NUM_COLS; c++) {
            if (buffer_row < SCROLL_BUFFER_ROWS) {
                buffer[r * NUM_COLS + c] = scroll_buffer[buffer_row][c];
            }
        }
    }

    // Update cursor position
    if (row >= scroll_offset && row < scroll_offset + NUM_ROWS) {
        size_t screen_row = row - scroll_offset;
        print_set_cursor(col, screen_row);
    }
}

void print_number(uint64_t num) {
    char buffer[21]; // Enough for 64-bit number plus null terminator
    int pos = 0;

    // Handle zero case
    if (num == 0) {
        print_char('0');
        return;
    }

    // Convert number to string (in reverse)
    while (num > 0 && pos < 20) {
        buffer[pos++] = '0' + (num % 10);
        num /= 10;
    }

    // Print in correct order
    while (pos > 0) {
        print_char(buffer[--pos]);
    }
}

// Add these new functions to the header file
void print_hex(uint8_t value) {
    const char hex_digits[] = "0123456789ABCDEF";
    print_char(hex_digits[(value >> 4) & 0xF]);
    print_char(hex_digits[value & 0xF]);
}
// void print_hex(uint64_t num) {
//     char hex_chars[] = "0123456789ABCDEF";
//     char buffer[17]; // 16 hex digits + null terminator
//     int pos = 0;

//     // Handle zero case
//     if (num == 0) {
//         print_str("0x0");
//         return;
//     }

//     // Convert to hex (in reverse)
//     while (num > 0 && pos < 16) {
//         buffer[pos++] = hex_chars[num & 0xF];
//         num >>= 4;
//     }

//     // Print prefix
//     print_str("0x");

//     // Print in correct order
//     while (pos > 0) {
//         print_char(buffer[--pos]);
//     }
// }

void print_bin(uint64_t num) {
    char buffer[65]; // 64 bits + null terminator
    int pos = 0;

    // Handle zero case
    if (num == 0) {
        print_str("0b0");
        return;
    }

    // Convert to binary (in reverse)
    while (num > 0 && pos < 64) {
        buffer[pos++] = '0' + (num & 1);
        num >>= 1;
    }

    // Print prefix
    print_str("0b");

    // Print in correct order
    while (pos > 0) {
        print_char(buffer[--pos]);
    }
}

void print_ip(uint32_t ip) {
    print_number((ip >> 24) & 0xFF);
    print_char('.');
    print_number((ip >> 16) & 0xFF);
    print_char('.');
    print_number((ip >> 8) & 0xFF);
    print_char('.');
    print_number(ip & 0xFF);
}

static const uint32_t vga_rgb_colors[] = {
    0x000000,  // BLACK
    0x0000AA,  // BLUE
    0x00AA00,  // GREEN
    0x00AAAA,  // CYAN
    0xAA0000,  // RED
    0xAA00AA,  // MAGENTA
    0xAA5500,  // BROWN
    0xAAAAAA,  // LIGHT_GRAY
    0x555555,  // DARK_GRAY
    0x5555FF,  // LIGHT_BLUE
    0x55FF55,  // LIGHT_GREEN
    0x55FFFF,  // LIGHT_CYAN
    0xFF5555,  // LIGHT_RED
    0xFF55FF,  // PINK
    0xFFFF55,  // YELLOW
    0xFFFFFF   // WHITE
};

// Calculate color distance (simple RGB distance)
static uint32_t color_distance(uint32_t c1, uint32_t c2) {
    int r1 = (c1 >> 16) & 0xFF;
    int g1 = (c1 >> 8) & 0xFF;
    int b1 = c1 & 0xFF;

    int r2 = (c2 >> 16) & 0xFF;
    int g2 = (c2 >> 8) & 0xFF;
    int b2 = c2 & 0xFF;

    int dr = r1 - r2;
    int dg = g1 - g2;
    int db = b1 - b2;

    return (dr * dr) + (dg * dg) + (db * db);
}

// Convert RGB color to closest VGA color
uint8_t print_rgb_to_vga(uint32_t rgb) {
    uint32_t min_distance = UINT32_MAX;
    uint8_t closest_color = 0;

    for (uint8_t i = 0; i < 16; i++) {
        uint32_t distance = color_distance(rgb, vga_rgb_colors[i]);
        if (distance < min_distance) {
            min_distance = distance;
            closest_color = i;
        }
    }

    return closest_color;
}

// Set colors using RGB values
void print_set_color_rgb(uint32_t fg_rgb, uint32_t bg_rgb) {
    uint8_t fg = print_rgb_to_vga(fg_rgb);
    uint8_t bg = print_rgb_to_vga(bg_rgb);
    print_set_color(fg, bg);
}

// Convert hex string to RGB value
static uint32_t hex_to_rgb(const char* hex) {
    // Skip '#' if present
    if (hex[0] == '#') hex++;

    uint32_t rgb = 0;
    for (int i = 0; i < 6; i++) {
        char c = hex[i];
        uint8_t val;

        if (c >= '0' && c <= '9') {
            val = c - '0';
        } else if (c >= 'a' && c <= 'f') {
            val = c - 'a' + 10;
        } else if (c >= 'A' && c <= 'F') {
            val = c - 'A' + 10;
        } else {
            return 0;  // Invalid hex character
        }

        rgb = (rgb << 4) | val;
    }

    return rgb;
}

// Set colors using hex strings
void print_set_color_hex(const char* fg_hex, const char* bg_hex) {
    uint32_t fg_rgb = hex_to_rgb(fg_hex);
    uint32_t bg_rgb = hex_to_rgb(bg_hex);
    print_set_color_rgb(fg_rgb, bg_rgb);
}

struct {
    size_t saved_col;
    size_t saved_row;
    uint8_t saved_color;
} print_state;

void print_save_state(void) {
    print_get_cursor(&print_state.saved_col, &print_state.saved_row);
    print_state.saved_color = color;
}

void print_restore_state(void) {
    print_set_cursor(print_state.saved_col, print_state.saved_row);
    print_set_color(print_state.saved_color, PRINT_COLOR_BLACK);
}

void print_set_scroll_offset(size_t offset) {
    if (offset > SCROLL_BUFFER_ROWS - NUM_ROWS) {
        offset = SCROLL_BUFFER_ROWS - NUM_ROWS;
    }
    scroll_offset = offset;
    print_refresh();
}

size_t print_get_scroll_offset(void) {
    return scroll_offset;
}

// Only refresh the parts that changed
static void refresh_line(size_t row) {
    size_t buffer_row = row + scroll_offset;
    for (size_t c = 0; c < NUM_COLS; c++) {
        buffer[c + NUM_COLS * row] = scroll_buffer[buffer_row][c];
    }
}
