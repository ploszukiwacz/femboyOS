#pragma once

#include <stdint.h>

extern volatile uint64_t tick_count;

// Initialize the timer system
void timer_init();

// Sleep for the specified number of milliseconds
void sleep(uint32_t ms);
