#include "mymalloc.h"
#include <stdio.h>
// from writeup
#define MEMSIZE 4096
static union {
    char bytes[MEMSIZE];
    double not_used;
} heap;

static int heap_initialized = 0;

void leak_detect_helper(void) { // detects leaks and leak size
    size_t num_bytes_leaked = 0;
    int num_blocks_leaked = 0;
    int chunk_number = 0;
    while (chunk_number < MEMSIZE) {
        size_t *current_head = (size_t*) &heap.bytes[chunk_number];
        size_t current_size = *current_head & ~7;
        if (current_size == 0) {
            break;
        }
        if (!(*current_head & 1)) { // if the block is not allocated
            num_bytes_leaked += current_size - sizeof(size_t);
            num_blocks_leaked++;
        }
        chunk_number += current_size;
    }
    if (num_blocks_leaked > 0) {
        fprintf(stderr, "mymalloc: %zu bytes leaked in %d objects.\n", num_bytes_leaked, num_blocks_leaked);
    }
}

size_t align(size_t size){ // aligns the size to the nearest multiple of 8
    return (size + sizeof(8) - 1) & ~(sizeof(8) - 1);
}

void* mymalloc(size_t size, char* file, int line) {
    if (!heap_initialized) {
        heap_initialized = 1;
        atexit(leak_detect_helper);
        size_t *initial_head = (size_t*) &heap.bytes[0];
        *initial_head = MEMSIZE; // set the initial head to the size of the heap
    }

    return NULL;
}