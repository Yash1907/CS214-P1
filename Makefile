CC = gcc
CFLAGS = -g -Wall -Wvla -fsanitize=address,undefined -std=c99

all: memgrind memtest correctness

memgrind: memgrind.c mymalloc.c mymalloc.h
	$(CC) $(CFLAGS) -o memgrind memgrind.c mymalloc.c

memtest: memtest.c mymalloc.c mymalloc.h
	$(CC) $(CFLAGS) -o memtest memtest.c mymalloc.c

correctness: correctness.c mymalloc.c mymalloc.h
	$(CC) $(CFLAGS) -o correctness correctness.c mymalloc.c

clean:
	rm -f memgrind memtest correctness

.PHONY: all clean
