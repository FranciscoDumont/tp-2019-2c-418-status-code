#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "libmuse.h"

int main() {
    uint32_t tamCpy = sizeof(int);
    uint32_t tamGet = sizeof(int);
    uint32_t dirCpy = muse_alloc(tamCpy);
    uint32_t dirGet = muse_alloc(tamGet);
    //dir = 24;
    printf("Estoy imprimiendo el valor de tamCpy: %d\n", tamCpy);
    printf("Estoy imprimiendo el valor de dirCpy: %d\n", dirCpy);
    //muse_free(dirCpy);
    //printf("Estoy imprimiendo el valor de dir: %d\n", dir);
    int resultMuseCpy = muse_cpy(tamCpy, &dirCpy, 4);
    printf("%d\n", resultMuseCpy);
    printf("%d\n", tamCpy);
    int resultMuseGet = muse_get(&tamGet, dirGet, 4);
    printf("%d\n", resultMuseGet);
    printf("%d\n", tamGet);

    return 0;
}