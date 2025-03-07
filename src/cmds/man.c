#include "man.h"
#include "../libs/print.h"

// Define all man pages here (in any order - we'll sort them before displaying)
static const man_page_t man_pages[] = {
    {
        "calc",
        "Simple calculator",
        "calc [expression]",
        "A basic calculator that can perform addition, subtraction, multiplication, and division.\n"
        "The expression should be in the format: number operator number\n"
        "Supported operators: +, -, *, /",
        "calc 5+3\n"
        "calc 10 - 4\n"
        "calc 6*8\n"
        "calc 20/4"
    },
    {
        "clear",
        "Clear the terminal screen",
        "clear",
        "Clears the screen, moving the cursor to the top-left corner.",
        "clear"
    },
    {
        "dance",
        "Display a dancing character animation",
        "dance",
        "Shows an animated dancing character. Press any key to stop the animation.",
        "dance"
    },
    {
        "echo",
        "Display a line of text",
        "echo [text]",
        "Displays the specified text on the terminal.\n"
        "This command doesn't support any special characters or variables.",
        "echo Hello, World!\n"
        "echo This is femboyOS"
    },
    {
        "fortune",
        "Display a random quote or message",
        "fortune",
        "Shows a random fortune message selected from a predefined list.",
        "fortune"
    },
    {
        "help",
        "Display help information about available commands",
        "help [command]",
        "Without arguments, 'help' displays a list of available commands.\n"
        "When a command is specified, it displays detailed help for that command.",
        "help\n"
        "help calc"
    },
    {
        "keytest",
        "Test keyboard input",
        "keytest",
        "Enters a keyboard testing mode that displays the code of each key pressed.\n"
        "This is useful for debugging keyboard input and verifying escape sequences.\n"
        "Press ESC key three times in a row to exit the test mode.",
        "keytest"
    },
    {
        "less",
        "View text with paging",
        "less [text]",
        "A text viewer that lets you scroll through text one page at a time.\n"
        "Use arrow keys to navigate and 'q' to quit.\n"
        "If no text is provided, it displays a usage message.",
        "less \"This is a long text that spans multiple lines\"\n"
        "echo \"Page 1\\nPage 2\\nPage 3\" | less"
    },
    {
        "man",
        "Manual page viewer",
        "man [command]",
        "Displays detailed documentation for the specified command.\n"
        "Use 'man' without arguments to see a list of all documented commands.",
        "man calc\n"
        "man help\n"
        "man man"
    },
    {
        "random",
        "Generate random numbers",
        "random [min] [max]",
        "Generates random numbers using a Linear Congruential Generator.\n"
        "Without arguments: generates a number between 0 and 100.\n"
        "With arguments: generates a number between min and max (inclusive).",
        "random\n"
        "random 1 6\n"
        "random 1000 9999"
    },
    {
        "poweroff",
        "Shut down the system",
        "poweroff",
        "Attempts to power off the system using various methods (ACPI, APM, and emulator-specific).\n"
        "Alias: shutdown",
        "poweroff"
    },
    {
        "refresh",
        "Refresh the terminal display",
        "refresh",
        "Forces a complete redraw of the terminal screen. Useful if display glitches occur.",
        "refresh"
    },
    {
        "restart",
        "Restart the system",
        "restart",
        "Attempts to restart the system using the keyboard controller or by forcing a triple fault.\n"
        "Alias: reboot",
        "restart"
    },
    {
        "sysfetch",
        "Display system information summary",
        "sysfetch",
        "Shows a summary of system information including OS version, kernel version,\n"
        "uptime, and hardware details.",
        "sysfetch"
    },
    {
        "tail",
        "Display the last lines of text",
        "tail [text]",
        "Displays the last 10 lines of the specified text.\n"
        "If no text is provided, it displays a usage message.",
        "tail \"Line 1\\nLine 2\\nLine 3\\nLine 4\"\n"
        "echo \"Multiple\\nLines\\nOf\\nText\" | tail"
    },
    {
        "terminal",
        "Terminal keyboard controls",
        "terminal",
        "The terminal in femboyOS supports the following keyboard shortcuts:\n"
        "\n"
        "  Arrow Left/Right - Move cursor within command line\n"
        "  Arrow Up/Down    - Navigate command history\n"
        "  Home             - Move to start of command\n"
        "  End              - Move to end of command\n"
        "  Insert           - Toggle insert/overwrite mode\n"
        "  Delete           - Remove character at cursor\n"
        "  Backspace        - Remove character before cursor",
        "Type a command and use arrows to edit it."
    },
    {
        "uptime",
        "Show how long the system has been running",
        "uptime",
        "Displays the time since the system was booted in hours, minutes, and seconds.",
        "uptime"
    },
    {
        "version",
        "Display system version information",
        "version",
        "Shows the current version of femboyOS and build information.",
        "version"
    },
    {
        "asm",
        "Execute raw x86_64 machine code",
        "asm [hex bytes]",
        "Executes machine code bytes provided in hexadecimal format.\n"
        "Each byte should be provided as two hex digits, separated by spaces.\n"
        "A return (ret) instruction 0xC3 is automatically appended if not present.\n"
        "\n"
        "WARNING: This command executes code directly with no safety checks.\n"
        "Invalid instructions may crash the system.",
        "asm 90           # Execute NOP\n"
        "asm 90 F4        # NOP followed by HLT\n"
        "asm 48 C7 C0 42 00 00 00    # mov rax, 42"
    },

    {
        "opcodes",
        "Common x86_64 machine code bytes reference",
        "opcodes",
        "Basic Instructions:\n"
        "  90       NOP          No operation\n"
        "  F4       HLT          Halt processor\n"
        "  C3       RET          Return from procedure\n"
        "  CC       INT3         Breakpoint\n"
        "\n"
        "System Instructions:\n"
        "  FA       CLI          Clear interrupt flag (disable interrupts)\n"
        "  FB       STI          Set interrupt flag (enable interrupts)\n"
        "  0F 05    SYSCALL      System call\n"
        "  CF       IRET         Interrupt return\n"
        "\n"
        "Common MOV Instructions:\n"
        "  48 C7 C0 ?? ?? ?? ??    MOV RAX, imm32    Move immediate to RAX\n"
        "  48 89 C3                MOV RBX, RAX     Move RAX to RBX\n"
        "  48 89 E5                MOV RBP, RSP     Setup stack frame\n"
        "\n"
        "Flag Instructions:\n"
        "  F8       CLC          Clear carry flag\n"
        "  F9       STC          Set carry flag\n"
        "  FC       CLD          Clear direction flag\n"
        "  FD       STD          Set direction flag\n"
        "\n"
        "String Instructions:\n"
        "  F3       REP          Repeat string operation\n"
        "  F2       REPNE        Repeat string operation while not equal\n"
        "  A4       MOVSB        Move byte string\n"
        "  A5       MOVSD        Move double word string\n"
        "\n"
        "Math Instructions:\n"
        "  48 01 D8    ADD RAX, RBX     Add RBX to RAX\n"
        "  48 29 D8    SUB RAX, RBX     Subtract RBX from RAX\n"
        "  48 F7 E3    MUL RBX          Unsigned multiply RAX by RBX\n"
        "  48 F7 FB    IDIV RBX         Signed divide RAX by RBX\n"
        "\n"
        "Jump Instructions:\n"
        "  EB ??    JMP rel8     Short jump (1 byte offset)\n"
        "  E9 ?? ?? ?? ??    JMP rel32    Near jump (4 byte offset)\n"
        "  74 ??    JE/JZ rel8   Jump if equal/zero\n"
        "  75 ??    JNE/JNZ rel8 Jump if not equal/not zero\n"
        "\n"
        "Stack Instructions:\n"
        "  50-57    PUSH reg     Push register (rax-rdi)\n"
        "  58-5F    POP reg      Pop register (rax-rdi)\n"
        "\n"
        "Note: ?? represents immediate values or offsets that need to be filled in.\n"
        "WARNING: Incorrect machine code can crash the system.",
        "# Push and pop RAX\n"
        "asm 50 58\n\n"
        "# Move 42 to RAX and add RBX\n"
        "asm 48 C7 C0 2A 00 00 00 48 01 D8\n\n"
        "# Simple loop (decrement RAX until zero)\n"
        "asm 48 C7 C0 05 00 00 00 48 FF C8 75 FB"
    }
};

// Export the number of man pages
const int man_page_count = sizeof(man_pages) / sizeof(man_pages[0]);

// Get a man page for a specific command
const man_page_t* man_get_page(const char* command) {
    for (int i = 0; i < man_page_count; i++) {
        // Simple string comparison
        const char* cmd = man_pages[i].command;
        const char* input = command;

        // Compare each character
        while (*cmd && *input && *cmd == *input) {
            cmd++;
            input++;
        }

        // If we reached the end of both strings, they're equal
        if (*cmd == 0 && *input == 0) {
            return &man_pages[i];
        }
    }

    return 0; // Not found
}

// Compare two strings for sorting
static int string_compare(const char* str1, const char* str2) {
    while (*str1 && *str1 == *str2) {
        str1++;
        str2++;
    }
    return *(const unsigned char*)str1 - *(const unsigned char*)str2;
}

// List all available man pages in alphabetical order
void CMD_man_list_all() {
    print_str("Available manual pages:\n\n");

    // Create an array of command indices for sorting
    int command_indices[man_page_count];
    for (int i = 0; i < man_page_count; i++) {
        command_indices[i] = i;
    }

    // Simple bubble sort by command name
    for (int i = 0; i < man_page_count - 1; i++) {
        for (int j = 0; j < man_page_count - i - 1; j++) {
            if (string_compare(man_pages[command_indices[j]].command,
                               man_pages[command_indices[j + 1]].command) > 0) {
                // Swap
                int temp = command_indices[j];
                command_indices[j] = command_indices[j + 1];
                command_indices[j + 1] = temp;
            }
        }
    }

    // Display commands in sorted order
    for (int i = 0; i < man_page_count; i++) {
        int idx = command_indices[i];
        print_str(man_pages[idx].command);
        print_str(" - ");
        print_str(man_pages[idx].short_desc);
        print_str("\n");
    }
}

void CMD_man(const char* command) {
    const char* topic = command + 4;

    // Check if the topic exists
    const man_page_t* page = man_get_page(topic);

    if (page) {
        // Print the man page
        print_set_color(PRINT_COLOR_WHITE, PRINT_COLOR_BLACK);
        print_str("NAME\n    ");
        print_str(page->command);
        print_str(" - ");
        print_str(page->short_desc);
        print_str("\n\n");

        print_str("SYNOPSIS\n    ");
        print_str(page->usage);
        print_str("\n\n");

        print_str("DESCRIPTION\n    ");
        print_str(page->long_desc);
        print_str("\n\n");

        print_str("EXAMPLES\n    ");
        print_str(page->examples);
        print_str("\n");
    } else {
        print_str("No manual entry for ");
        print_str(topic);
        print_str("\n");
    }
}
