#pragma once

#include <stdint.h>

void port_byte_out(uint16_t port, uint8_t data);
void port_word_out(uint16_t port, uint16_t data);
void port_dword_out(uint16_t port, uint32_t data);

uint8_t port_byte_in(uint16_t port);
uint16_t port_word_in(uint16_t port);
uint32_t port_dword_in(uint16_t port);

uint8_t inb(uint16_t port);
void outb(uint16_t port, uint8_t data);