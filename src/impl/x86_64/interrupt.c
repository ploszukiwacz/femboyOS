#include "../../intf/interrupt.h"
#include "../../intf/port.h"
#include "../../intf/pic.h"

#define IDT_ENTRIES 256

// The IDT - an array of interrupt descriptors
idt_entry_t idt[IDT_ENTRIES];
idtr_t idtr;

// Array of interrupt handlers
isr_t interrupt_handlers[IDT_ENTRIES];

// External assembly functions
extern void isr0();
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr9();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();
extern void isr32();  // Timer interrupt
extern void isr33();
extern void isr34();
extern void isr35();
extern void isr36();
extern void isr37();
extern void isr38();
extern void isr39();
extern void isr40();
extern void isr41();
extern void isr42();
extern void isr43();
extern void isr44();
extern void isr45();
extern void isr46();
extern void isr47();

// Function to set an entry in the IDT
static void idt_set_gate(uint8_t num, uint64_t base, uint16_t selector, uint8_t flags) {
    idt[num].offset_1 = base & 0xFFFF;
    idt[num].offset_2 = (base >> 16) & 0xFFFF;
    idt[num].offset_3 = (base >> 32) & 0xFFFFFFFF;
    idt[num].selector = selector;
    idt[num].ist = 0;
    idt[num].type_attributes = flags;
    idt[num].zero = 0;
}

// Initialize the IDT
void idt_init() {
    idtr.limit = sizeof(idt_entry_t) * IDT_ENTRIES - 1;
    idtr.base = (uint64_t)&idt;

    // Clear out the IDT
    for (int i = 0; i < IDT_ENTRIES; i++) {
        idt_set_gate(i, 0, 0, 0);
        interrupt_handlers[i] = 0;
    }

    // Set up ISR gates (CPU exceptions)
    idt_set_gate(0, (uint64_t)isr0, 0x08, 0x8E);
    idt_set_gate(1, (uint64_t)isr1, 0x08, 0x8E);
    idt_set_gate(2, (uint64_t)isr2, 0x08, 0x8E);
    idt_set_gate(3, (uint64_t)isr3, 0x08, 0x8E);
    idt_set_gate(4, (uint64_t)isr4, 0x08, 0x8E);
    idt_set_gate(5, (uint64_t)isr5, 0x08, 0x8E);
    idt_set_gate(6, (uint64_t)isr6, 0x08, 0x8E);
    idt_set_gate(7, (uint64_t)isr7, 0x08, 0x8E);
    idt_set_gate(8, (uint64_t)isr8, 0x08, 0x8E);
    idt_set_gate(9, (uint64_t)isr9, 0x08, 0x8E);
    idt_set_gate(10, (uint64_t)isr10, 0x08, 0x8E);
    idt_set_gate(11, (uint64_t)isr11, 0x08, 0x8E);
    idt_set_gate(12, (uint64_t)isr12, 0x08, 0x8E);
    idt_set_gate(13, (uint64_t)isr13, 0x08, 0x8E);
    idt_set_gate(14, (uint64_t)isr14, 0x08, 0x8E);
    idt_set_gate(15, (uint64_t)isr15, 0x08, 0x8E);
    idt_set_gate(16, (uint64_t)isr16, 0x08, 0x8E);
    idt_set_gate(17, (uint64_t)isr17, 0x08, 0x8E);
    idt_set_gate(18, (uint64_t)isr18, 0x08, 0x8E);
    idt_set_gate(19, (uint64_t)isr19, 0x08, 0x8E);
    idt_set_gate(20, (uint64_t)isr20, 0x08, 0x8E);
    idt_set_gate(21, (uint64_t)isr21, 0x08, 0x8E);
    idt_set_gate(22, (uint64_t)isr22, 0x08, 0x8E);
    idt_set_gate(23, (uint64_t)isr23, 0x08, 0x8E);
    idt_set_gate(24, (uint64_t)isr24, 0x08, 0x8E);
    idt_set_gate(25, (uint64_t)isr25, 0x08, 0x8E);
    idt_set_gate(26, (uint64_t)isr26, 0x08, 0x8E);
    idt_set_gate(27, (uint64_t)isr27, 0x08, 0x8E);
    idt_set_gate(28, (uint64_t)isr28, 0x08, 0x8E);
    idt_set_gate(29, (uint64_t)isr29, 0x08, 0x8E);
    idt_set_gate(30, (uint64_t)isr30, 0x08, 0x8E);
    idt_set_gate(31, (uint64_t)isr31, 0x08, 0x8E);

    // Set up IRQ handlers (32-47)
    idt_set_gate(32, (uint64_t)isr32, 0x08, 0x8E);  // Timer
    idt_set_gate(33, (uint64_t)isr33, 0x08, 0x8E);  // Keyboard
    idt_set_gate(34, (uint64_t)isr34, 0x08, 0x8E);
    idt_set_gate(35, (uint64_t)isr35, 0x08, 0x8E);
    idt_set_gate(36, (uint64_t)isr36, 0x08, 0x8E);
    idt_set_gate(37, (uint64_t)isr37, 0x08, 0x8E);
    idt_set_gate(38, (uint64_t)isr38, 0x08, 0x8E);
    idt_set_gate(39, (uint64_t)isr39, 0x08, 0x8E);
    idt_set_gate(40, (uint64_t)isr40, 0x08, 0x8E);
    idt_set_gate(41, (uint64_t)isr41, 0x08, 0x8E);
    idt_set_gate(42, (uint64_t)isr42, 0x08, 0x8E);
    idt_set_gate(43, (uint64_t)isr43, 0x08, 0x8E);
    idt_set_gate(44, (uint64_t)isr44, 0x08, 0x8E);
    idt_set_gate(45, (uint64_t)isr45, 0x08, 0x8E);
    idt_set_gate(46, (uint64_t)isr46, 0x08, 0x8E);
    idt_set_gate(47, (uint64_t)isr47, 0x08, 0x8E);

    // Load the IDT
    __asm__ volatile("lidt %0" : : "m"(idtr));
}

// Remap the PIC to use interrupts 32-47
void pic_remap() {
    // Initialize command
    port_byte_out(PIC1_COMMAND, 0x11);
    port_byte_out(PIC2_COMMAND, 0x11);

    // Set vector offsets
    port_byte_out(PIC1_DATA, 0x20); // PIC1 -> 32-39
    port_byte_out(PIC2_DATA, 0x28); // PIC2 -> 40-47

    // Tell PICs about each other
    port_byte_out(PIC1_DATA, 0x04);
    port_byte_out(PIC2_DATA, 0x02);

    // Set 8086 mode
    port_byte_out(PIC1_DATA, 0x01);
    port_byte_out(PIC2_DATA, 0x01);

    // Mask all interrupts except timer (IRQ0)
    port_byte_out(PIC1_DATA, 0xFC); // 0xFC = 11111100, IRQ0 and IRQ1 unmasked
    port_byte_out(PIC2_DATA, 0xFF); // All IRQs on PIC2 masked
}

// Initialize interrupts
void interrupt_init() {
    // Initialize IDT
    idt_init();

    // Remap PICs
    pic_remap();
}

// Register an interrupt handler
void register_interrupt_handler(uint8_t n, isr_t handler) {
    interrupt_handlers[n] = handler;
}

// Enable interrupts
void enable_interrupts() {
    __asm__ volatile("sti");
}

// Disable interrupts
void disable_interrupts() {
    __asm__ volatile("cli");
}

// Generic ISR handler
void isr_handler(uint64_t interrupt_number) {
    // First send EOI to avoid missing interrupts
    if (interrupt_number >= 32 && interrupt_number < 48) {
        if (interrupt_number >= 40) {
            // If this came from the slave PIC (IRQ8-15), send EOI to it too
            port_byte_out(PIC2_COMMAND, PIC_EOI);
        }
        // Always send EOI to master PIC (IRQ0-7 and cascaded IRQ8-15)
        port_byte_out(PIC1_COMMAND, PIC_EOI);
    }

    // Then call the handler
    if (interrupt_handlers[interrupt_number] != 0) {
        isr_t handler = interrupt_handlers[interrupt_number];
        handler();
    }
}
