#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include "mymalloc.h"

#define MEMLENGTH 4096
#define ALIGNMENT 8
#define HEADER_SIZE sizeof(size_t)
#define MIN_CHUNK_SIZE (HEADER_SIZE + ALIGNMENT)

static union {
    char bytes[MEMLENGTH];
    double not_used;
} heap;

static int heap_initialized = 0;

// Header encoding:
//   bit 0: 1 = allocated, 0 = free
//   bits 3+: chunk size (always a multiple of 8, so low 3 bits are available)
//
// Chunk layout: [header (8 bytes)][payload (>= 8 bytes)]
// The pointer returned to the client points to the payload.

static size_t align8(size_t size) {
    return (size + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1);
}

static void leak_detect(void) {
    size_t leaked_bytes = 0;
    int leaked_objects = 0;
    size_t offset = 0;

    while (offset < MEMLENGTH) {
        size_t *header = (size_t *)&heap.bytes[offset];
        size_t chunk_size = *header & ~7;
        if (chunk_size == 0)
            break;
        if (*header & 1) { // allocated = leaked
            leaked_bytes += chunk_size - HEADER_SIZE;
            leaked_objects++;
        }
        offset += chunk_size;
    }

    if (leaked_objects > 0) {
        fprintf(stderr, "mymalloc: %zu bytes leaked in %d objects.\n",
                leaked_bytes, leaked_objects);
    }
}

static void init_heap(void) {
    size_t *header = (size_t *)&heap.bytes[0];
    *header = MEMLENGTH; // entire heap is one free chunk (bit 0 = 0)
    heap_initialized = 1;
    atexit(leak_detect);
}

void *mymalloc(size_t size, char *file, int line) {
    if (!heap_initialized)
        init_heap();

    if (size == 0)
        return NULL;

    size_t payload_size = align8(size);
    if (payload_size < ALIGNMENT)
        payload_size = ALIGNMENT;
    size_t needed = payload_size + HEADER_SIZE;

    size_t offset = 0;
    while (offset < MEMLENGTH) {
        size_t *header = (size_t *)&heap.bytes[offset];
        size_t chunk_size = *header & ~7;
        int allocated = *header & 1;

        if (!allocated && chunk_size >= needed) {
            size_t leftover = chunk_size - needed;
            if (leftover >= MIN_CHUNK_SIZE) {
                // split: allocate first part, leave remainder free
                *header = needed | 1;
                size_t *next_header = (size_t *)&heap.bytes[offset + needed];
                *next_header = leftover; // free (bit 0 = 0)
            } else {
                // give the whole chunk (don't create a too-small remainder)
                *header = chunk_size | 1;
            }
            return (void *)&heap.bytes[offset + HEADER_SIZE];
        }
        offset += chunk_size;
    }

    fprintf(stderr, "malloc: Unable to allocate %zu bytes (%s:%d)\n", size, file, line);
    return NULL;
}

void myfree(void *ptr, char *file, int line) {
    if (!heap_initialized)
        init_heap();

    if (ptr == NULL)
        return;

    char *p = (char *)ptr;

    // Check that the pointer is within the heap payload area
    if (p < heap.bytes + HEADER_SIZE || p >= heap.bytes + MEMLENGTH) {
        fprintf(stderr, "free: Inappropriate pointer (%s:%d)\n", file, line);
        exit(2);
    }

    // Walk the chunk list to validate the pointer
    size_t offset = 0;
    while (offset < MEMLENGTH) {
        size_t *header = (size_t *)&heap.bytes[offset];
        size_t chunk_size = *header & ~7;
        if (chunk_size == 0)
            break;

        char *payload = &heap.bytes[offset + HEADER_SIZE];
        if (payload == p) {
            // Found the chunk. Check if it's currently allocated.
            if (!(*header & 1)) {
                // Double free
                fprintf(stderr, "free: Inappropriate pointer (%s:%d)\n", file, line);
                exit(2);
            }
            // Mark as free
            *header = chunk_size; // bit 0 = 0

            // Coalesce with next chunk if it's free
            size_t next_offset = offset + chunk_size;
            if (next_offset < MEMLENGTH) {
                size_t *next_header = (size_t *)&heap.bytes[next_offset];
                size_t next_size = *next_header & ~7;
                if (next_size > 0 && !(*next_header & 1)) {
                    // Merge: absorb the next chunk
                    chunk_size += next_size;
                    *header = chunk_size; // still free
                }
            }

            // Coalesce with previous chunk if it's free
            // Walk from the beginning to find the predecessor
            if (offset > 0) {
                size_t prev_offset = 0;
                size_t scan = 0;
                while (scan < offset) {
                    prev_offset = scan;
                    size_t *sh = (size_t *)&heap.bytes[scan];
                    scan += *sh & ~7;
                }
                size_t *prev_header = (size_t *)&heap.bytes[prev_offset];
                if (!(*prev_header & 1)) {
                    // Previous chunk is free; merge
                    size_t prev_size = *prev_header & ~7;
                    size_t merged = prev_size + (*header & ~7);
                    *prev_header = merged; // still free
                }
            }
            return;
        }

        offset += chunk_size;
    }

    // Pointer not at the start of any chunk
    fprintf(stderr, "free: Inappropriate pointer (%s:%d)\n", file, line);
    exit(2);
}
