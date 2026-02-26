#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef REALMALLOC
#include "mymalloc.h"
#endif

#define MEMSIZE 4096
#define HEADERSIZE 8

void test_free_deallocates() {
    void *p = malloc(100);
    free(p);
    void *q = malloc(100);
    if (q == NULL) {
        printf("FAIL test_free_deallocates: could not re-malloc after free\n");
    } else {
        printf("PASS test_free_deallocates\n");
        free(q);
    }
}

void test_no_overlap() {
    int errors = 0;
    char *a = malloc(64);
    char *b = malloc(64);
    memset(a, 1, 64);
    memset(b, 2, 64);
    for (int i = 0; i < 64; i++) {
        if (a[i] != 1) errors++;
    }
    if (errors > 0) {
        printf("FAIL test_no_overlap: %d corrupted bytes\n", errors);
    } else {
        printf("PASS test_no_overlap\n");
    }
    free(a);
    free(b);
}

void test_coalesce() {
    char *a = malloc(100);
    char *b = malloc(100);
    char *c = malloc(100);  
    free(a);
    free(b);
    char *d = malloc(150);
    if (d == NULL) {
        printf("FAIL test_coalesce: could not allocate from coalesced space\n");
    } else {
        printf("PASS test_coalesce\n");
        free(d);
    }
    free(c);
}

void test_out_of_memory() {
    void *ptrs[256];
    int i;
    for (i = 0; i < 256; i++) {
        ptrs[i] = malloc(32);
        if (ptrs[i] == NULL) break;
    }
    if (ptrs[i] == NULL) {
        printf("PASS test_out_of_memory: malloc returned NULL when heap exhausted\n");
    } else {
        printf("FAIL test_out_of_memory: malloc never returned NULL\n");
    }
    for (int j = 0; j < i; j++) {
        free(ptrs[j]);
    }
}

int main() {
    test_free_deallocates();
    test_no_overlap();
    test_coalesce();
    test_out_of_memory();
    return 0;
}