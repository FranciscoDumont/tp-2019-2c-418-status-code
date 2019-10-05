#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "libmuse.h"

int main() {
    uint32_t tam = sizeof(int);
    uint32_t dir = muse_alloc(tam);
    //dir = 24;
    printf("Estoy imprimiendo el valor de tam: %d\n", tam);
    printf("Estoy imprimiendo el valor de dir: %d\n", dir);
    muse_free(dir);
    //printf("Estoy imprimiendo el valor de dir: %d\n", dir);
    int result; // = muse_cpy(tam, &dir, 4);
    printf("%d\n", result =  muse_cpy(tam, &dir, 4));
    printf("%d\n", tam);

    return 0;
}