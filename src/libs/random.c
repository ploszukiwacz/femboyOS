#include "random.h"

// LCG parameters (same as glibc)
#define LCG_MULTIPLIER 1103515245
#define LCG_INCREMENT 12345
#define LCG_MODULUS 0x80000000

// Current random seed
static uint32_t random_state = 1;

void random_init(void) {
    // Use the timer tick count as initial seed
    extern volatile uint64_t tick_count;
    random_seed((uint32_t)tick_count);
}

void random_seed(uint32_t seed) {
    random_state = seed;
}

uint32_t random_next(void) {
    // Linear Congruential Generator formula:
    // X_(n+1) = (a * X_n + c) mod m
    random_state = (LCG_MULTIPLIER * random_state + LCG_INCREMENT) % LCG_MODULUS;
    return random_state;
}

uint32_t random_range(uint32_t min, uint32_t max) {
    // Ensure max > min
    if (max <= min) {
        return min;
    }

    uint32_t range = max - min + 1;
    uint32_t raw = random_next();

    // Scale the random number to our range and add minimum
    return min + (raw % range);
}
