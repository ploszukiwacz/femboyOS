#include "keyboard.h"
#include "interrupt.h"
#include "port.h"
#include "pic.h"

#define KEYBOARD_DATA_PORT     0x60
#define KEYBOARD_STATUS_PORT   0x64
#define KEYBOARD_COMMAND_PORT  0x64
#define KEYBOARD_BUFFER_SIZE   256

// Keyboard buffer (circular buffer)
static volatile char keyboard_buffer[KEYBOARD_BUFFER_SIZE];
static volatile uint32_t keyboard_buffer_write = 0;
static volatile uint32_t keyboard_buffer_read = 0;

// Flag to track if the keyboard is initialized
static bool keyboard_initialized = false;

// US keyboard layout scancode to ASCII conversion
static const char scancode_to_ascii[] = {
    0, 0x1B, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0, ' ', 0
};

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
#define EXTENDED_KEY     0xE0

static bool shift_pressed = false;
static bool caps_lock_on = false;
static bool extended_key = false;

// Wait for keyboard controller to be ready
static void keyboard_wait() {
    int timeout = 100000;
    while ((port_byte_in(KEYBOARD_STATUS_PORT) & 0x02) && --timeout) {
        __asm__ volatile("pause");
    }
}

// Wait for keyboard data
static void keyboard_wait_data() {
    int timeout = 100000;
    while (!(port_byte_in(KEYBOARD_STATUS_PORT) & 0x01) && --timeout) {
        __asm__ volatile("pause");
    }
}

// Add a character to the keyboard buffer
static void keyboard_buffer_add(char c) {
    if (c == 0) return;

    uint32_t next_write = (keyboard_buffer_write + 1) % KEYBOARD_BUFFER_SIZE;

    // Only add if buffer isn't full
    if (next_write != keyboard_buffer_read) {
        keyboard_buffer[keyboard_buffer_write] = c;
        keyboard_buffer_write = next_write;
    }
}

// Reset the keyboard controller
static void keyboard_reset() {
    // Disable keyboard
    keyboard_wait();
    port_byte_out(KEYBOARD_COMMAND_PORT, 0xAD);

    // Flush keyboard buffer
    while (port_byte_in(KEYBOARD_STATUS_PORT) & 0x01) {
        port_byte_in(KEYBOARD_DATA_PORT);
    }

    // Enable keyboard
    keyboard_wait();
    port_byte_out(KEYBOARD_COMMAND_PORT, 0xAE);

    // Reset settings
    keyboard_wait();
    port_byte_out(KEYBOARD_DATA_PORT, 0xFF);

    // Wait for ACK
    keyboard_wait_data();
    port_byte_in(KEYBOARD_DATA_PORT);
}

// Keyboard interrupt handler
static void keyboard_callback() {
    // Read the keyboard status
    uint8_t status = port_byte_in(KEYBOARD_STATUS_PORT);

    // Check if there's data to read
    if (!(status & 0x01)) return;

    // Read the scancode
    uint8_t scancode = port_byte_in(KEYBOARD_DATA_PORT);

    // Handle extended keys
    if (scancode == EXTENDED_KEY) {
        extended_key = true;
        return;
    }

    if (extended_key) {
        extended_key = false;

        // Handle extended keys here
        switch (scancode) {
            case 0x48: keyboard_buffer_add(KEY_ARROW_UP); break;
            case 0x50: keyboard_buffer_add(KEY_ARROW_DOWN); break;
            case 0x4B: keyboard_buffer_add(KEY_ARROW_LEFT); break;
            case 0x4D: keyboard_buffer_add(KEY_ARROW_RIGHT); break;
            case 0x47: keyboard_buffer_add(KEY_HOME); break;
            case 0x4F: keyboard_buffer_add(KEY_END); break;
            case 0x53: keyboard_buffer_add(KEY_DELETE); break;
            case 0x52: keyboard_buffer_add(KEY_INSERT); break;
            case 0x49: keyboard_buffer_add(KEY_PGUP); break;
            case 0x51: keyboard_buffer_add(KEY_PGDN); break;
        }
        return;
    }

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

    // Ignore key releases (top bit set)
    if (scancode & 0x80) return;

    // Convert scancode to ASCII
    if (scancode < sizeof(scancode_to_ascii)) {
        char ascii;
        if (shift_pressed) {
            ascii = scancode_to_ascii_shift[scancode];
        } else {
            ascii = scancode_to_ascii[scancode];
        }

        // Apply caps lock (only to letters)
        if (caps_lock_on) {
            if (ascii >= 'a' && ascii <= 'z') {
                ascii = ascii - 'a' + 'A';
            } else if (ascii >= 'A' && ascii <= 'Z') {
                ascii = ascii - 'A' + 'a';
            }
        }

        keyboard_buffer_add(ascii);
    }
}

void keyboard_init() {
    if (keyboard_initialized) return;

    // Reset the keyboard controller
    keyboard_reset();

    // Clear the buffer
    keyboard_buffer_write = 0;
    keyboard_buffer_read = 0;

    // Register keyboard interrupt handler (IRQ1 maps to interrupt 33)
    register_interrupt_handler(33, keyboard_callback);

    // Unmask keyboard interrupt in PIC
    uint8_t current_mask = port_byte_in(PIC1_DATA);
    port_byte_out(PIC1_DATA, current_mask & ~(1 << 1));

    keyboard_initialized = true;
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
        return 0;
    }

    char c = keyboard_buffer[keyboard_buffer_read];
    keyboard_buffer_read = (keyboard_buffer_read + 1) % KEYBOARD_BUFFER_SIZE;
    return c;
}
