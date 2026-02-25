#include <stdlib.h>
#include "mymalloc.h"

int main() {
    malloc(50);
    malloc(100);
    // no free — leak detector should report 2 objects at exit
    return 0;
}