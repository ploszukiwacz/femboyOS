#pragma once

#include <stdint.h>

void port_byte_out(uint16_t port, uint8_t data);
uint8_t port_byte_in(uint16_t port);
