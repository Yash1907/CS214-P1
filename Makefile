CC = gcc
CFLAGS = -Wall -g

all: memgrind memtest correctness test_bad_pointer test_bad_offset test_double_free test_leak

memgrind: memgrind.c mymalloc.c
	$(CC) $(CFLAGS) -o memgrind memgrind.c mymalloc.c

memtest: memtest.c mymalloc.c
	$(CC) $(CFLAGS) -o memtest memtest.c mymalloc.c

correctness: correctness.c mymalloc.c
	$(CC) $(CFLAGS) -o correctness correctness.c mymalloc.c

test_bad_pointer: test_bad_pointer.c mymalloc.c
	$(CC) $(CFLAGS) -o test_bad_pointer test_bad_pointer.c mymalloc.c

test_bad_offset: test_bad_offset.c mymalloc.c
	$(CC) $(CFLAGS) -o test_bad_offset test_bad_offset.c mymalloc.c

test_double_free: test_double_free.c mymalloc.c
	$(CC) $(CFLAGS) -o test_double_free test_double_free.c mymalloc.c

test_leak: test_leak.c mymalloc.c
	$(CC) $(CFLAGS) -o test_leak test_leak.c mymalloc.c

clean:
	rm -f memgrind memtest correctness test_bad_pointer test_bad_offset test_double_free test_leak
	rm -rf *.dSYM