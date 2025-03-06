#include "keyboard.h"
#include "interrupt.h"
#include "port.h"
#include "pic.h"

#define KEYBOARD_DATA_PORT     0x60
#define KEYBOARD_STATUS_PORT   0x64
#define KEYBOARD_BUFFER_SIZE   256

// Keyboard buffer (circular buffer)
static volatile char keyboard_buffer[KEYBOARD_BUFFER_SIZE];
static volatile uint32_t keyboard_buffer_write = 0;
static volatile uint32_t keyboard_buffer_read = 0;

// US keyboard layout scancode to ASCII conversion
static const char scancode_to_ascii[] = {
    0, 0x1B, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0, ' ', 0
};

// Shift keys mapping (for uppercase letters and special characters)
static const char scancode_to_ascii_shift[] = {
    0, 0, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',
    0, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,
    '*', 0, ' ', 0
};

// Control keys
#define LSHIFT_PRESS     0x2A
#define RSHIFT_PRESS     0x36
#define LSHIFT_RELEASE   0xAA
#define RSHIFT_RELEASE   0xB6
#define CAPS_LOCK        0x3A

static bool shift_pressed = false;
static bool caps_lock_on = false;

// Add a character to the keyboard buffer
static void keyboard_buffer_add(char c) {
    // Only add printable characters, tab, newline, and backspace
    if (c == 0)
        return;

    uint32_t next_write = (keyboard_buffer_write + 1) % KEYBOARD_BUFFER_SIZE;

    // Check if buffer is full
    if (next_write != keyboard_buffer_read) {
        keyboard_buffer[keyboard_buffer_write] = c;
        keyboard_buffer_write = next_write;
    }
}

// Keyboard interrupt handler
static void keyboard_callback() {
    // Read the keyboard data port to get the scancode
    uint8_t scancode = port_byte_in(KEYBOARD_DATA_PORT);

    // Handle shift keys
    if (scancode == LSHIFT_PRESS || scancode == RSHIFT_PRESS) {
        shift_pressed = true;
        return;
    } else if (scancode == LSHIFT_RELEASE || scancode == RSHIFT_RELEASE) {
        shift_pressed = false;
        return;
    }

    // Handle caps lock
    if (scancode == CAPS_LOCK) {
        caps_lock_on = !caps_lock_on;
        return;
    }

    // Handle key release (scancodes >= 0x80)
    if (scancode & 0x80)
        return;

    // Convert scancode to ASCII
    char ascii = 0;
    if (scancode < sizeof(scancode_to_ascii)) {
        if (shift_pressed) {
            ascii = scancode_to_ascii_shift[scancode];
        } else {
            ascii = scancode_to_ascii[scancode];
        }

        // Apply caps lock (only to letters)
        if (caps_lock_on && ascii >= 'a' && ascii <= 'z') {
            ascii = ascii - 'a' + 'A';
        } else if (caps_lock_on && ascii >= 'A' && ascii <= 'Z') {
            ascii = ascii - 'A' + 'a';
        }

        // Add the key to the buffer
        keyboard_buffer_add(ascii);
    }
}

void keyboard_init() {
    // Register keyboard interrupt handler (IRQ1 maps to interrupt 33)
    register_interrupt_handler(33, keyboard_callback);

    // Unmask keyboard interrupt in PIC
    uint8_t current_mask = port_byte_in(PIC1_DATA);
    port_byte_out(PIC1_DATA, current_mask & ~(1 << 1)); // Clear bit 1 to unmask IRQ1
}

bool keyboard_is_key_available() {
    return keyboard_buffer_read != keyboard_buffer_write;
}

char keyboard_read() {
    // Wait until a key is available
    while (keyboard_buffer_read == keyboard_buffer_write) {
        __asm__ volatile("hlt");
    }

    // Read the key from the buffer
    char c = keyboard_buffer[keyboard_buffer_read];
    keyboard_buffer_read = (keyboard_buffer_read + 1) % KEYBOARD_BUFFER_SIZE;
    return c;
}

char keyboard_read_nonblocking() {
    if (keyboard_buffer_read == keyboard_buffer_write) {
        return 0; // No key available
    }

    // Read the key from the buffer
    char c = keyboard_buffer[keyboard_buffer_read];
    keyboard_buffer_read = (keyboard_buffer_read + 1) % KEYBOARD_BUFFER_SIZE;
    return c;
}

char keyboard_wait_for_key() {
    return keyboard_read();
}
