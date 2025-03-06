#include "../../intf/print.h"

const static size_t NUM_COLS = 80;
const static size_t NUM_ROWS = 25;

struct Char {
    uint8_t character;
    uint8_t color;
};

struct Char* buffer = (struct Char*) 0xb8000;
size_t col = 0;
size_t row = 0;
uint8_t color = PRINT_COLOR_WHITE | PRINT_COLOR_BLACK << 4;

void clear_row(size_t row) {
    struct Char empty = (struct Char) {
        .character = ' ',
        .color = color,
    };

    for (size_t col = 0; col < NUM_COLS; col++) {
        buffer[col + NUM_COLS * row] = empty;
    }
}

void print_clear() {
    // Save current color
    uint8_t saved_color = color;

    for (size_t i = 0; i < NUM_ROWS; i++) {
        clear_row(i);
    }

    // Reset cursor position
    col = 0;
    row = 0;

    // Restore color
    color = saved_color;
}

void print_newline() {
    col = 0;

    if (row < NUM_ROWS - 1) {
        row++;
        return;
    }

    // Scroll the screen up by one line
    for (size_t r = 1; r < NUM_ROWS; r++) {
        for (size_t c = 0; c < NUM_COLS; c++) {
            buffer[c + NUM_COLS * (r-1)] = buffer[c + NUM_COLS * r];
        }
    }

    // Clear the last row
    clear_row(NUM_ROWS - 1);
}

void print_char(char character) {
    if (character == '\n') {
        print_newline();
        return;
    }

    if (character == '\b') {
        // Handle backspace - move cursor back and print a space
        if (col > 0) {
            col--;
            buffer[col + NUM_COLS * row].character = ' ';
        }
        return;
    }

    if (col >= NUM_COLS) {
        print_newline();
    }

    buffer[col + NUM_COLS * row] = (struct Char) {
        .character = (uint8_t) character,
        .color = color,
    };

    col++;
}

void print_str(const char* str) {
    for (size_t i = 0; 1; i++) {
        char character = (uint8_t) str[i];

        if (character == '\0') {
            return;
        }

        print_char(character);
    }
}

void print_set_color(uint8_t foreground, uint8_t background) {
    color = foreground + (background << 4);
}

// Add a function to get the cursor position
void print_get_cursor(size_t* out_col, size_t* out_row) {
    *out_col = col;
    *out_row = row;
}

// Add a function to set the cursor position
void print_set_cursor(size_t new_col, size_t new_row) {
    if (new_col < NUM_COLS && new_row < NUM_ROWS) {
        col = new_col;
        row = new_row;
    }
}

// Add to print.c
void print_refresh() {
    // Get the current state
    size_t old_col = col;
    size_t old_row = row;
    uint8_t old_color = color;

    // Force a complete redraw of the screen
    struct Char* video_mem = (struct Char*) 0xb8000;
    for (size_t r = 0; r < NUM_ROWS; r++) {
        for (size_t c = 0; c < NUM_COLS; c++) {
            // Read and write back each character (refresh memory)
            struct Char ch = video_mem[c + NUM_COLS * r];
            video_mem[c + NUM_COLS * r] = ch;
        }
    }

    // Restore state
    col = old_col;
    row = old_row;
    color = old_color;
}
