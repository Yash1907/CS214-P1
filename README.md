Names and NetIDs: Yash Shastri (ys1204) and Ansh Jetli (aj1180)
Note that in order to use the make file to compile the code you must do:
```
make all
```
We wrote 2 test files:
`correctness.c` and `memgrind.c`.
The purpose of `correctness.c` is to test the correctness of our mymalloc and myfree functions, namely that they work as intended without error.
The purpose of `memgrind.c` is slightly different, it tests the performance of our functions and how it performs under load.
There are multiple ways to compile both the files:
```
gcc -o correctness correctness.c mymalloc.c
gcc -o memgrind memgrind.c mymalloc.c
```
You could also use the real malloc instead for comparison:
```
gcc -DREALMALLOC -o memgrind memgrind.c
```
The best way however is to use the make file:
```
make all
```
Or individually:
```
make correctnes
make memgrind
```
# Testing plan
Note that neither of the programs take in command line arguments.
For `correctness.c`:
We made 4 tests to verify that our mymalloc and myfree functions worked as expected.
1. We tested using `test_free_deallocates` that the memory can be reused after being freed.
2. We tested using `test_no_overlap` that we were able to allocate 2 blocks of memory without messing up the other.
3. We tested using `test_coalesce` that adjacent freed blocks were coalesced into a larger chunk.
4. We tested using `test_out_of_memory` that our mymalloc returns NULL when there is no memory left for it to allocate.
For `memgrind.c`:
We made 5 tests to verify that our functions could handle stress and load.
1. For test 1 we malloc(1) and freed it immediately 120 times.
2. For test 2 we allocated 120 blocks of size 1 and then freed them all.
3. For test 3 we did a random alloc/free till 120 allocations had occured.
4. For test 4 we made 20 linked list nodes, traversed the list, and then freed all the nodes.
5. For test 5 we allocated 20 blocks of different sizes and freed them in reverse order.
The idea behind this was to try to mimic potential real usage and to see if our functions would work as expected under stress.