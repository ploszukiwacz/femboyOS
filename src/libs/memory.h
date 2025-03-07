#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

// Initialize memory management
void memory_init(void);

// Allocate memory
void* kalloc(size_t size);

// Free memory
void kfree(void* ptr);

// Allocate aligned memory
void* kalloc_aligned(size_t size, size_t alignment);

// Get total memory size
size_t memory_get_total(void);

// Get free memory size
size_t memory_get_free(void);
