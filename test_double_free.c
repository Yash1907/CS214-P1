#include <stdlib.h>
#include "mymalloc.h"

int main() {
    int *p = malloc(sizeof(int));
    free(p);
    free(p);  // should print error and exit(2)
    return 0;
}