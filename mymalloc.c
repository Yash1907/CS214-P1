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
        size_t *initial_head = (size_t*) &heap.bytes;
        *initial_head = MEMSIZE|0; // set the initial head to the size of the heap
    }
    if (size == 0) {
        return NULL;
    }
    int curr_chunk = 0;
    size_t padded_size = align(size);
    size_t total_memory_needed = padded_size + sizeof(size_t);
    while (curr_chunk < MEMSIZE) {
        size_t *curr_header = (size_t *)&heap.bytes[curr_chunk];
        size_t curr_size = *curr_header & ~7;
        int empty = *curr_header & 1;
        if (empty && curr_size >= total_memory_needed){
            size_t excess_memory = curr_size - total_memory_needed;
            if (excess_memory >= (sizeof(size_t) + 8)){ // splitting the header if excess memory is big enough
                *curr_header = total_memory_needed|1;
                size_t *new_header = (size_t *)&heap.bytes[curr_chunk + total_memory_needed];
                *new_header = excess_memory|0;
            }
            else{
                *curr_header = curr_size|1;
            }
            return (void*)&heap.bytes[curr_chunk + sizeof(size_t)];
        }
        curr_chunk += curr_size;
    }
    fprintf(stderr, "malloc: Unable to allocate %zu bytes (%s:%d)\n", size, file, line);
    return NULL;
}