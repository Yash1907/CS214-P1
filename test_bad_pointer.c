#include <stdlib.h>
#include "mymalloc.h"

int main() {
    int x;
    free(&x);  // should print error and exit(2)
    return 0;
}