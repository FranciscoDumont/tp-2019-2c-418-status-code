#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

/**
     * Reserva una porción de memoria contígua de tamaño `tam`.
     * @param tam La cantidad de bytes a reservar.
     * @return La dirección de la memoria reservada.
     */
uint32_t muse_alloc(uint32_t tam) {
    uint32_t *reserved = malloc(sizeof(tam));
    return *reserved;
}

/**
    * Libera una porción de memoria reservada.
    * @param dir La dirección de la memoria a reservar.
    */
void muse_free(uint32_t dir) {
    free(dir);
}

/**
    * Copia una cantidad `n` de bytes desde una posición de memoria de MUSE a una `dst` local.
    * @param dst Posición de memoria local con tamaño suficiente para almacenar `n` bytes.
    * @param src Posición de memoria de MUSE de donde leer los `n` bytes.
    * @param n Cantidad de bytes a copiar.
    * @return Si pasa un error, retorna -1. Si la operación se realizó correctamente, retorna 0.
    */
int muse_get(void* dst, uint32_t src, size_t n) {

}
