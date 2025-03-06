#include "print.h"
#include "timer.h"
#include "interrupt.h"
#include "keyboard.h"
#include "cli.h"
#include "port.h"

void kernel_main() {
    print_clear();
    print_set_color(PRINT_COLOR_WHITE, PRINT_COLOR_BLACK);
    print_str("femboyOS loading...\n");

    // Initialize subsystems
    interrupt_init();
    timer_init();
    keyboard_init();
    enable_interrupts();

    print_str("System initialized!\n");
    sleep(500);

    // Initialize and run the command line interface
    cli_init();
    cli_run();

    // This code should never be reached
    while(1) {
        __asm__ volatile("hlt");
    }
}
