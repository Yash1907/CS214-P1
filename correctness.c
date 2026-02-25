#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#include "mymalloc.h"

static int tests_run = 0;
static int tests_passed = 0;

static void report(const char *name, int passed) {
    tests_run++;
    if (passed) {
        tests_passed++;
        printf("  PASS: %s\n", name);
    } else {
        printf("  FAIL: %s\n", name);
    }
}

// Run a function in a child process and check its exit status.
// Returns the exit status of the child (2 = expected error exit).
static int run_in_child(void (*fn)(void)) {
    pid_t pid = fork();
    if (pid == 0) {
        // child
        fn();
        _exit(0);
    }
    int status;
    waitpid(pid, &status, 0);
    if (WIFEXITED(status))
        return WEXITSTATUS(status);
    return -1;
}

// ---------- Test: basic allocation and data integrity ----------
static void test_alloc_integrity(void) {
    printf("Test: allocation integrity\n");
    #define NOBJS 64
    #define OBJSZ 48
    char *objs[NOBJS];
    int ok = 1;

    for (int i = 0; i < NOBJS; i++) {
        objs[i] = malloc(OBJSZ);
        if (objs[i] == NULL) {
            ok = 0;
            break;
        }
    }
    report("allocate 64 objects", ok);

    // fill each with distinct pattern
    for (int i = 0; i < NOBJS; i++)
        memset(objs[i], i, OBJSZ);

    // verify patterns are intact (no overlap)
    ok = 1;
    for (int i = 0; i < NOBJS; i++) {
        for (int j = 0; j < OBJSZ; j++) {
            if ((unsigned char)objs[i][j] != (unsigned char)i) {
                ok = 0;
                break;
            }
        }
        if (!ok) break;
    }
    report("no overlap between allocated objects", ok);

    for (int i = 0; i < NOBJS; i++)
        free(objs[i]);
    #undef NOBJS
    #undef OBJSZ
}

// ---------- Test: free deallocates memory (can reuse) ----------
static void test_free_reuse(void) {
    printf("Test: free makes memory reusable\n");

    // fill up memory with large objects
    void *a = malloc(2000);
    void *b = malloc(2000);
    report("allocate two large objects", a != NULL && b != NULL);

    // should fail - not enough room
    void *c = malloc(2000);
    report("third large alloc fails (out of memory)", c == NULL);

    // free one and try again
    free(a);
    void *d = malloc(2000);
    report("alloc succeeds after freeing", d != NULL);

    free(b);
    free(d);
}

// ---------- Test: coalescing adjacent free chunks ----------
static void test_coalesce(void) {
    printf("Test: coalescing adjacent free chunks\n");

    // allocate three chunks that together fill most of memory
    void *a = malloc(1000);
    void *b = malloc(1000);
    void *c = malloc(1000);
    report("allocate three 1000-byte objects", a && b && c);

    // free all three
    free(a);
    free(b);
    free(c);

    // if coalescing works, we should be able to allocate a single large chunk
    void *big = malloc(3500);
    report("large alloc succeeds after coalescing", big != NULL);

    free(big);
}

// ---------- Test: free with non-heap pointer exits with status 2 ----------
static void child_free_stack_ptr(void) {
    int x;
    free(&x);
}

static void test_free_bad_pointer(void) {
    printf("Test: free with bad pointer\n");
    int status = run_in_child(child_free_stack_ptr);
    report("free(stack pointer) exits with status 2", status == 2);
}

// ---------- Test: free with pointer into middle of chunk ----------
static void child_free_middle(void) {
    int *p = malloc(sizeof(int) * 4);
    free(p + 1);
}

static void test_free_middle_pointer(void) {
    printf("Test: free with middle-of-chunk pointer\n");
    int status = run_in_child(child_free_middle);
    report("free(p+1) exits with status 2", status == 2);
}

// ---------- Test: double free ----------
static void child_double_free(void) {
    int *p = malloc(sizeof(int));
    free(p);
    free(p);
}

static void test_double_free(void) {
    printf("Test: double free\n");
    int status = run_in_child(child_double_free);
    report("double free exits with status 2", status == 2);
}

// ---------- Test: malloc(0) returns NULL ----------
static void test_malloc_zero(void) {
    printf("Test: malloc(0)\n");
    void *p = malloc(0);
    report("malloc(0) returns NULL", p == NULL);
}

// ---------- Test: alignment ----------
static void test_alignment(void) {
    printf("Test: alignment\n");
    int ok = 1;
    void *ptrs[10];
    for (int i = 0; i < 10; i++) {
        ptrs[i] = malloc(1 + (i * 7)); // varying sizes
        if (ptrs[i] == NULL || ((size_t)ptrs[i] % 8) != 0) {
            ok = 0;
        }
    }
    report("all returned pointers are 8-byte aligned", ok);
    for (int i = 0; i < 10; i++)
        free(ptrs[i]);
}

// ---------- Test: leak detection ----------
// The leak detector prints to stderr at exit. We can fork and check.
static void child_leak(void) {
    malloc(100);
    malloc(200);
    // intentionally don't free - should trigger leak message at exit
}

static void test_leak_detection(void) {
    printf("Test: leak detection\n");
    // The child will print to stderr and exit 0 (leak detector doesn't call exit)
    int status = run_in_child(child_leak);
    report("leaking program exits normally (status 0)", status == 0);
}

// ---------- Test: coalesce with previous free chunk ----------
static void test_coalesce_backward(void) {
    printf("Test: backward coalescing\n");
    void *a = malloc(500);
    void *b = malloc(500);
    void *c = malloc(500);
    report("allocate a, b, c", a && b && c);

    // free a first, then b -> b should merge with a
    free(a);
    free(b);

    // now free c -> c should merge with the a+b block
    free(c);

    // should be able to allocate nearly the whole heap
    void *big = malloc(4000);
    report("big alloc after backward coalesce succeeds", big != NULL);
    free(big);
}

int main(int argc, char **argv) {
    printf("=== mymalloc correctness tests ===\n\n");

    test_alloc_integrity();
    test_free_reuse();
    test_coalesce();
    test_free_bad_pointer();
    test_free_middle_pointer();
    test_double_free();
    test_malloc_zero();
    test_alignment();
    test_leak_detection();
    test_coalesce_backward();

    printf("\n%d/%d tests passed.\n", tests_passed, tests_run);
    return (tests_passed == tests_run) ? EXIT_SUCCESS : EXIT_FAILURE;
}
