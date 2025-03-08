#include "panic.h"
#include "../libs/print.h"

// Convert integer to string
static void int_to_str(int num, char* str) {
    int i = 0;
    int is_negative = 0;

    if (num == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return;
    }

    if (num < 0) {
        is_negative = 1;
        num = -num;
    }

    while (num != 0) {
        str[i++] = (num % 10) + '0';
        num = num / 10;
    }

    if (is_negative) {
        str[i++] = '-';
    }

    str[i] = '\0';

    // Reverse the string
    int start = 0;
    int end = i - 1;
    while (start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }
}

// Convert uint32_t to hex string
static void uint32_to_hex_str(uint32_t num, char* str) {
    const char* hex_digits = "0123456789ABCDEF";
    for (int i = 0; i < 8; i++) {
        str[7 - i] = hex_digits[num & 0xF];
        num >>= 4;
    }
    str[8] = '\0';
}

noreturn void panic(const char* message, const char* file, int line) {
    // Save all registers immediately upon entering panic
    uint32_t eax, ebx, ecx, edx, esi, edi, ebp, esp;

    // Capture registers using inline assembly
    asm volatile (
        "movl %%eax, %0\n\t"
        "movl %%ebx, %1\n\t"
        "movl %%ecx, %2\n\t"
        "movl %%edx, %3\n\t"
        "movl %%esi, %4\n\t"
        "movl %%edi, %5\n\t"
        "movl %%ebp, %6\n\t"
        "movl %%esp, %7"
        : "=m" (eax), "=m" (ebx), "=m" (ecx), "=m" (edx),
          "=m" (esi), "=m" (edi), "=m" (ebp), "=m" (esp)
        :
        : "memory"
    );

    // Disable interrupts
    asm volatile("cli");

    // Clear screen and set panic colors (white on red)
    print_clear();
    print_set_color(PRINT_COLOR_WHITE, PRINT_COLOR_RED);

    // Print panic message with fancy formatting
    print_str("\n\n");
    print_str("**************************************\n");
    print_str("*           KERNEL PANIC             *\n");
    print_str("**************************************\n\n");

    // Error details
    print_str("Error: ");
    print_str(message);
    print_str("\n\n");

    print_str("File: ");
    print_str(file);
    print_str("\n");

    print_str("Line: ");
    char line_str[32];
    int_to_str(line, line_str);
    print_str(line_str);
    print_str("\n\n");

    // Register dump with better formatting
    print_str("Register Dump:\n");
    print_str("************************************\n");

    char hex_str[9];

    // First row
    print_str("EAX: 0x");
    uint32_to_hex_str(eax, hex_str);
    print_str(hex_str);
    print_str("  EBX: 0x");
    uint32_to_hex_str(ebx, hex_str);
    print_str(hex_str);
    print_str("\n");

    // Second row
    print_str("ECX: 0x");
    uint32_to_hex_str(ecx, hex_str);
    print_str(hex_str);
    print_str("  EDX: 0x");
    uint32_to_hex_str(edx, hex_str);
    print_str(hex_str);
    print_str("\n");

    // Third row
    print_str("ESI: 0x");
    uint32_to_hex_str(esi, hex_str);
    print_str(hex_str);
    print_str("  EDI: 0x");
    uint32_to_hex_str(edi, hex_str);
    print_str(hex_str);
    print_str("\n");

    // Fourth row
    print_str("EBP: 0x");
    uint32_to_hex_str(ebp, hex_str);
    print_str(hex_str);
    print_str("  ESP: 0x");
    uint32_to_hex_str(esp, hex_str);
    print_str(hex_str);
    print_str("\n");

    print_str("\n");
    print_str("System halted. Power off the machine.\n");

    // Infinite halt loop
    while (1) {
        asm volatile("hlt");
    }
}
