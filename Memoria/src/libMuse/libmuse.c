#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <commons/memory.h>

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
//TODO preguntar si esta bien esto, creo que no lo esta
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
    copying_to_memory(&dst, src, n);
}

/**
     * Copia una cantidad `n` de bytes desde una posición de memoria local a una `dst` en MUSE.
     * @param dst Posición de memoria de MUSE con tamaño suficiente para almacenar `n` bytes.
     * @param src Posición de memoria local de donde leer los `n` bytes.
     * @param n Cantidad de bytes a copiar.
     * @return Si pasa un error, retorna -1. Si la operación se realizó correctamente, retorna 0.
     */
int muse_cpy(uint32_t dst, void* src, int n) {
    copying_to_memory(dst, &src, n);
}

/**
 *Copiando a memoria
 * @param dst Posición de memoria a la que copia
 * @param src Posición de memoria copiada
 * @param n Cantidad de bytes a copiar
 * @return Si pasa un error, retorna -1. Si la operación se realizó correctamente, retorna 0.
 */
int copying_to_memory(uint32_t dst, uint32_t src, int n) {
    if(memcpy(&dst, &src, n) == &dst) {
        return 0;
    } else {
        return -1;
    }
}