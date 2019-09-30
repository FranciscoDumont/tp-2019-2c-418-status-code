#include <stdio.h>
#include "libmuse.h"

int main() {
    printf("Hello, World!\n");
    printf(&muse_alloc);

    return 0;
}