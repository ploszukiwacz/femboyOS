#pragma once

#include <stdint.h>

// Structure for interrupt descriptor
typedef struct {
    uint16_t offset_1;        // offset bits 0..15
    uint16_t selector;        // a code segment selector in GDT or LDT
    uint8_t  ist;             // bits 0..2 holds Interrupt Stack Table offset, rest of bits zero
    uint8_t  type_attributes; // gate type, dpl, and p fields
    uint16_t offset_2;        // offset bits 16..31
    uint32_t offset_3;        // offset bits 32..63
    uint32_t zero;            // reserved
} __attribute__((packed)) idt_entry_t;

typedef struct {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed)) idtr_t;

// Function pointer type for interrupt handlers
typedef void (*isr_t)(void);

// Initialize the interrupt system
void interrupt_init();

// Register a handler for a specific interrupt
void register_interrupt_handler(uint8_t n, isr_t handler);

// Enable interrupts
void enable_interrupts();

// Disable interrupts
void disable_interrupts();
