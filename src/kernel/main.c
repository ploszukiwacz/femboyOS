#include "../libs/print.h"
#include "../libs/timer.h"
#include "../libs/interrupt.h"
#include "../libs/keyboard.h"
#include "../cmds/command_registry.h"
// #include "../libs/net/ethernet.h"
// #include "../libs/net/ip.h"
// #include "../libs/net/icmp.h"
// #include "../libs/types.h"
#include "cli.h"
#include "panic.h"

void kernel_main(uint32_t multiboot_magic, void* multiboot_info) {
    print_clear();
    // print_set_color(PRINT_COLOR_WHITE, PRINT_COLOR_BLACK);
    print_set_color_rgb(0xFF55FF, 0x000000);
    print_str("femboyOS loading... (This may take a couple of seconds)\n");

    void (*init_functions[])() = {
        interrupt_init,
        timer_init,
        keyboard_init,
        enable_interrupts,
        initialize_command_registry
    };

    print_str("[");
    for (int i = 0; i < (int)(sizeof(init_functions) / sizeof(init_functions[0])); i++) {
        print_str("#");
        init_functions[i]();
    }
    print_str("] Done\n");



    // Network support comming in 2030 - ploszukiwacz
    // print_str("Checking for network hardware...");
    //     bool net_available = false;

    //     // Set a short timeout for network detection
    //     uint64_t start_time = tick_count;
    //     net_available = ethernet_init();
    //     if (!net_available) {
    //         print_str("not detected\n");
    //     } else {
    //         ip_init(0xC0A80164);  // 192.168.1.100
    //         icmp_init();
    //         print_str("initialized\n");
    //     }

    // Initialize and run the command line interface
    cli_init();
    cli_run();

    PANIC("Failed to init the cli.");

    // This code should never be reached
    // while(1) {
    //     __asm__ volatile("hlt");
    // }
}
