#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "mymalloc.h"

#define RUNS 50
#define NUM_ALLOCS 120

// Task 1: malloc and immediately free a 1-byte object, 120 times.
static void task1(void) {
    for (int i = 0; i < NUM_ALLOCS; i++) {
        void *p = malloc(1);
        free(p);
    }
}

// Task 2: malloc 120 1-byte objects, store pointers, then free all.
static void task2(void) {
    void *ptrs[NUM_ALLOCS];
    for (int i = 0; i < NUM_ALLOCS; i++) {
        ptrs[i] = malloc(1);
    }
    for (int i = 0; i < NUM_ALLOCS; i++) {
        free(ptrs[i]);
    }
}

// Task 3: Randomly choose between allocating a 1-byte object and freeing
// a random previously allocated object. Once 120 allocations have been made,
// free all remaining objects.
static void task3(void) {
    void *ptrs[NUM_ALLOCS];
    int count = 0;       // current number of live allocations
    int total = 0;       // total number of allocations made

    while (total < NUM_ALLOCS) {
        if (count == 0 || (rand() % 2 == 0 && total < NUM_ALLOCS)) {
            ptrs[count] = malloc(1);
            count++;
            total++;
        } else {
            int idx = rand() % count;
            free(ptrs[idx]);
            ptrs[idx] = ptrs[count - 1];
            count--;
        }
    }
    // free remaining
    for (int i = 0; i < count; i++) {
        free(ptrs[i]);
    }
}

// Task 4: Simulate a LIFO stack - push and pop varying-size allocations.
// Allocate objects of random sizes (1-16 bytes) up to a limit, then pop
// them all in reverse order (LIFO).
static void task4(void) {
    void *stack[NUM_ALLOCS];
    int top = 0;

    for (int i = 0; i < NUM_ALLOCS; i++) {
        size_t sz = (rand() % 16) + 1;
        stack[top++] = malloc(sz);
    }
    // free in reverse order (LIFO)
    while (top > 0) {
        free(stack[--top]);
    }
}

// Task 5: Simulate a linked-list style workload. Repeatedly build a
// "linked list" of small nodes, traverse them, then tear it down.
// Do this 10 times with lists of 12 nodes each.
static void task5(void) {
    struct node {
        int value;
        int next_idx; // index into nodes array, -1 = end
    };

    for (int round = 0; round < 10; round++) {
        struct node *nodes[12];
        // build the list
        for (int i = 0; i < 12; i++) {
            nodes[i] = malloc(sizeof(struct node));
            nodes[i]->value = i * round;
            nodes[i]->next_idx = (i + 1 < 12) ? i + 1 : -1;
        }
        // traverse and use data (prevent optimization)
        volatile int sum = 0;
        int idx = 0;
        while (idx != -1) {
            sum += nodes[idx]->value;
            idx = nodes[idx]->next_idx;
        }
        // tear down
        for (int i = 0; i < 12; i++) {
            free(nodes[i]);
        }
    }
}

int main(int argc, char **argv) {
    struct timeval start, end;
    void (*tasks[])(void) = {task1, task2, task3, task4, task5};
    const char *names[] = {
        "Task 1: malloc/free 120 times",
        "Task 2: malloc 120 then free 120",
        "Task 3: random malloc/free pattern",
        "Task 4: LIFO stack with random sizes",
        "Task 5: linked list build/traverse/teardown"
    };

    srand(42); // deterministic seed for reproducibility

    for (int t = 0; t < 5; t++) {
        gettimeofday(&start, NULL);
        for (int r = 0; r < RUNS; r++) {
            tasks[t]();
        }
        gettimeofday(&end, NULL);

        long elapsed_us = (end.tv_sec - start.tv_sec) * 1000000L
                        + (end.tv_usec - start.tv_usec);
        double avg_us = (double)elapsed_us / RUNS;
        printf("%s: Average time = %.2f us\n", names[t], avg_us);
    }

    return EXIT_SUCCESS;
}
