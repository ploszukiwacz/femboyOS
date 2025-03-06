#include "timer.h"
#include "port.h"
#include "interrupt.h"

// PIT (Programmable Interval Timer) operates at 1.19 MHz
#define PIT_FREQUENCY 1193182
#define PIT_DATA_PORT 0x40
#define PIT_COMMAND_PORT 0x43

volatile uint64_t tick_count = 0;

// ISR for timer (IRQ0, which is mapped to interrupt 32)
void timer_callback() {
    tick_count++;

    // Read from a safe I/O port to keep timer alive
    // Keyboard status port (0x64) is generally safe to read
    port_byte_in(0x64);
}

void timer_init() {
    // Set up PIT to generate interrupts at approximately 1000Hz (1ms intervals)
    uint32_t divisor = PIT_FREQUENCY / 1000;

    // Send command byte
    port_byte_out(PIT_COMMAND_PORT, 0x36); // 0x36 = channel 0, access mode: lobyte/hibyte, mode 3, binary

    // Send divisor
    port_byte_out(PIT_DATA_PORT, divisor & 0xFF);         // Low byte
    port_byte_out(PIT_DATA_PORT, (divisor >> 8) & 0xFF);  // High byte

    // Register the timer callback for IRQ0 (interrupt 32)
    register_interrupt_handler(32, timer_callback);
}

void sleep(uint32_t ms) {
    uint64_t start_tick = tick_count;
    uint64_t target_tick = tick_count + ms;

    while (tick_count < target_tick) {
        // Read from an I/O port in the sleep loop as well
        if ((tick_count & 0xF) == 0) {  // Every 16 ticks
            port_byte_in(0x64);  // Keyboard status port
        }

        // Halt the CPU until next interrupt
        __asm__ volatile("hlt");
    }
}
