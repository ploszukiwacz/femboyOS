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
