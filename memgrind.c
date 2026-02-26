#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>

#ifndef REALMALLOC
#include "mymalloc.h"
#endif

#define RUNS 50

void task1() {
    for (int i = 0; i < 120; i++) {
        void *p = malloc(1);
        free(p);
    }
}

void task2() {
    void *ptrs[120];
    for (int i = 0; i < 120; i++) {
        ptrs[i] = malloc(1);
    }
    for (int i = 0; i < 120; i++) {
        free(ptrs[i]);
    }
}

void task3() {
    void *ptrs[120];
    int total = 0;
    int active = 0;

    memset(ptrs, 0, sizeof(ptrs));

    while (total < 120) {
        if (active == 0 || (rand() % 2 == 0 && total < 120)) {
            ptrs[active++] = malloc(1);
            total++;
        } else {
            int idx = rand() % active;
            free(ptrs[idx]);
            ptrs[idx] = ptrs[--active];
        }
    }
    for (int i = 0; i < active; i++) {
        free(ptrs[i]);
    }
}

typedef struct Node {
    int value;
    struct Node *next;
} Node;

void task4() {
    Node *head = NULL;
    Node *tail = NULL;

    for (int i = 0; i < 20; i++) {
        Node *n = malloc(sizeof(Node));
        n->value = i;
        n->next = NULL;
        if (head == NULL) {
            head = tail = n;
        } else {
            tail->next = n;
            tail = n;
        }
    }
    Node *cur = head;
    while (cur != NULL) {
        cur = cur->next;
    }
    cur = head;
    while (cur != NULL) {
        Node *next = cur->next;
        free(cur);
        cur = next;
    }
}

void task5() {
    void *ptrs[20];
    int sizes[] = {8, 16, 24, 32, 48, 64, 8, 16, 24, 32,
                   48, 64, 8, 16, 24, 32, 48, 64, 8, 16};

    for (int i = 0; i < 20; i++) {
        ptrs[i] = malloc(sizes[i]);
    }
    for (int i = 19; i >= 0; i--) {
        free(ptrs[i]);
    }
}

void run_task(void (*task)(), int task_num) {
    struct timeval start, end;
    gettimeofday(&start, NULL);
    for (int i = 0; i < RUNS; i++) {
        task();
    }
    gettimeofday(&end, NULL);

    long usec = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec);
    printf("Task %d average time: %.2f microseconds\n", task_num, (double)usec / RUNS);
}

int main() {
    run_task(task1, 1);
    run_task(task2, 2);
    run_task(task3, 3);
    run_task(task4, 4);
    run_task(task5, 5);
    return 0;
}