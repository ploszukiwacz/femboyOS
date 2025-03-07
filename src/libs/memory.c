#include "memory.h"

// Memory block header structure
typedef struct block_header {
    size_t size;                  // Size of the block (including header)
    bool is_free;                // Is this block free?
    struct block_header* next;    // Next block in the list
} block_header_t;

// Memory manager state
static struct {
    void* heap_start;            // Start of heap
    void* heap_end;              // End of heap
    block_header_t* first_block; // First block in the list
    size_t total_size;          // Total heap size
    size_t used_size;           // Used memory size
} mm;

// Align address up to alignment
static uintptr_t align_up(uintptr_t addr, size_t alignment) {
    return (addr + (alignment - 1)) & ~(alignment - 1);
}

void memory_init(void) {
    // For now, use a fixed memory region
    // In a real OS, you'd get this from the bootloader
    mm.heap_start = (void*)0x100000;  // Start at 1MB
    mm.heap_end = (void*)0x500000;    // End at 5MB
    mm.total_size = (size_t)(mm.heap_end - mm.heap_start);
    mm.used_size = 0;

    // Initialize first block
    mm.first_block = (block_header_t*)mm.heap_start;
    mm.first_block->size = mm.total_size;
    mm.first_block->is_free = true;
    mm.first_block->next = NULL;
}

void* kalloc(size_t size) {
    if (size == 0) return NULL;

    // Add header size to allocation
    size_t total_size = size + sizeof(block_header_t);

    // Find a free block
    block_header_t* current = mm.first_block;
    while (current) {
        if (current->is_free && current->size >= total_size) {
            // Split block if it's too large
            if (current->size > total_size + sizeof(block_header_t) + 16) {
                block_header_t* new_block = (block_header_t*)((uint8_t*)current + total_size);
                new_block->size = current->size - total_size;
                new_block->is_free = true;
                new_block->next = current->next;

                current->size = total_size;
                current->next = new_block;
            }

            current->is_free = false;
            mm.used_size += current->size;

            // Return pointer to data area
            return (void*)((uint8_t*)current + sizeof(block_header_t));
        }
        current = current->next;
    }

    return NULL;  // No suitable block found
}

void* kalloc_aligned(size_t size, size_t alignment) {
    if (size == 0 || alignment == 0) return NULL;

    // Allocate extra space for alignment and header
    size_t total_size = size + alignment + sizeof(block_header_t);

    void* ptr = kalloc(total_size);
    if (!ptr) return NULL;

    // Find aligned address
    uintptr_t aligned_addr = align_up((uintptr_t)ptr, alignment);

    // If already aligned, return as is
    if ((void*)aligned_addr == ptr) return ptr;

    // Adjust the block
    block_header_t* header = (block_header_t*)((uint8_t*)ptr - sizeof(block_header_t));
    size_t adjustment = aligned_addr - (uintptr_t)ptr;

    // Create new header at aligned position
    block_header_t* aligned_header = (block_header_t*)(aligned_addr - sizeof(block_header_t));
    aligned_header->size = header->size - adjustment;
    aligned_header->is_free = false;
    aligned_header->next = header->next;

    // Update original header
    header->size = adjustment;
    header->next = aligned_header;

    return (void*)aligned_addr;
}

void kfree(void* ptr) {
    if (!ptr) return;

    // Get block header
    block_header_t* header = (block_header_t*)((uint8_t*)ptr - sizeof(block_header_t));
    header->is_free = true;
    mm.used_size -= header->size;

    // Merge with next block if it's free
    while (header->next && header->next->is_free) {
        header->size += header->next->size;
        header->next = header->next->next;
    }

    // Find previous block and merge if it's free
    block_header_t* prev = mm.first_block;
    while (prev && prev->next != header) {
        prev = prev->next;
    }

    if (prev && prev->is_free) {
        prev->size += header->size;
        prev->next = header->next;
    }
}

size_t memory_get_total(void) {
    return mm.total_size;
}

size_t memory_get_free(void) {
    return mm.total_size - mm.used_size;
}
