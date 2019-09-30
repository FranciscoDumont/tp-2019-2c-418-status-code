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
    uint32_t *concat = malloc(sizeof(tam));
    return *concat;
}

/**
    * Libera una porción de memoria reservada.
    * @param dir La dirección de la memoria a reservar.
    */
void muse_free(uint32_t dir) {
    free(dir);
}