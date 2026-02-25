#include <stdlib.h>
#include "mymalloc.h"

int main() {
    int *p = malloc(sizeof(int) * 2);
    free(p + 1);  // should print error and exit(2)
    return 0;
}