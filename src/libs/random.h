#pragma once

#include <stdint.h>
#include <stdbool.h>

// Initialize the random number generator
void random_init(void);

// Get a random number between min and max (inclusive)
uint32_t random_range(uint32_t min, uint32_t max);

// Get a raw random number
uint32_t random_next(void);

// Set the random seed manually
void random_seed(uint32_t seed);
