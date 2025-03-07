#include "port.h"

void port_byte_out(uint16_t port, uint8_t data) {
    __asm__("outb %0, %1" : : "a"(data), "Nd"(port));
}

void port_word_out(uint16_t port, uint16_t data) {
    __asm__ volatile("outw %0, %1" : : "a"(data), "Nd"(port));
}

void port_dword_out(uint16_t port, uint32_t data) {
    __asm__ volatile("outl %0, %1" : : "a"(data), "Nd"(port));
}

uint8_t port_byte_in(uint16_t port) {
    uint8_t result;
    __asm__("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

uint16_t port_word_in(uint16_t port) {
    uint16_t result;
    __asm__ volatile("inw %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

uint32_t port_dword_in(uint16_t port) {
    uint32_t result;
    __asm__ volatile("inl %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}
