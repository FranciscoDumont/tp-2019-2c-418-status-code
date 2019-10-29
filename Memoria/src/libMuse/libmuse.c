#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <commons/memory.h>
#include "libmuse.h"

char* MUSE_IP;
int MUSE_PUERTO;

/**
 * Inicializa la biblioteca de MUSE.
 * @param id El Process (o Thread) ID para identificar el caller en MUSE.
 * @param ip IP en dot-notation de MUSE.
 * @param puerto Puerto de conexión a MUSE.
 * @return Si pasa un error, retorna -1. Si se inicializó correctamente, retorna 0.
 * @see Para obtener el id de un proceso pueden usar getpid() de la lib POSIX (unistd.h)
 * @note Debido a la naturaleza centralizada de MUSE, esta función deberá definir
 *  el ID del proceso/hilo según "IP-ID".
 */

int validate_create_sockets() {
    if((server_socket = create_socket()) == -1) {
        printf("Error al crear el socket\n");
        return -1;
    }
}

int validate_connect_sockets(char* ip, int puerto) {
    if(connect_socket(server_socket, ip, puerto) == -1){
        printf("Error al conectarse al servidor\n");
        return -1;
    }
}

void set_global_vars(char* ip, int puerto) {
    MUSE_IP = ip;
    MUSE_PUERTO = puerto;
    printf("Seteado MUSE_IP: %s\n", MUSE_IP);
    printf("Seteado MUSE_PUERTO: %d\n", MUSE_PUERTO);
}

void element_destroyer(void* element){
    free(element);
}

void free_after_send(void* _id, t_paquete *package, MessageHeader* buffer_header, t_list *respuesta_list){
    free_package(package);
    free(_id);
    free(buffer_header);
    list_destroy_and_destroy_elements(respuesta_list, element_destroyer);
}

int validate_send_package(void* _id, t_paquete *package) {
    if(send_package(package, server_socket) == -1){
        printf("Error al enviar paquete\n");
        return -1;
    } else {
        // Espero la respuesta del server diciendo que todo está ok
        MessageHeader* buffer_header = malloc(sizeof(MessageHeader));
        receive_header(server_socket, buffer_header);
        t_list *respuesta_list = receive_package(server_socket, buffer_header);
        int respuesta = *((int*)list_get(respuesta_list, 0));
        free_after_send(_id, package, buffer_header, respuesta_list);
        //Devuelvo la respuesta
        return respuesta;
    }
}

int muse_init(int id, char* ip, int puerto){
    //Creo y conecto sockets
    validate_create_sockets();
    validate_connect_sockets(ip, puerto);
    //Setteo las variables globales
    set_global_vars(ip, puerto);
    //Creo y aniado al paquete
    t_paquete *package = create_package(MUSE_INIT);
    void* _id = malloc(sizeof(int));
    *((int*)_id) = id;
    add_to_package(package, _id, sizeof(int));
    //Envio paquete
    return validate_send_package(_id, package);
}


/**
     * Reserva una porción de memoria contígua de tamaño `tam`.
     * @param tam La cantidad de bytes a reservar.
     * @return La dirección de la memoria reservada.
     */
uint32_t muse_alloc(uint32_t tam) {
    // Creo socket
    if((server_socket = create_socket()) == -1) {
        printf("Error al crear el socket\n");
        return -1;
    }

    // Conecto sockets
    if(connect_socket(server_socket, MUSE_IP, MUSE_PUERTO) == -1){
        printf("Error al conectarse al servidor\n");
        return -1;
    }

    //Creo un paquete para el Alloc
    t_paquete *package = create_package(MUSE_ALLOC);

    //Agrego el uint32_t al paquete
    void* tam_aux = malloc(sizeof(uint32_t));
    *((uint32_t*)tam_aux) = tam;
    add_to_package(package, tam_aux, sizeof(int));

    //Envio el paquete
    if(send_package(package, server_socket) == -1){
        printf("Error al enviar paquete\n");
        return -1;
    } else {
        // Espero la respuesta del server
        MessageHeader* buffer_header = malloc(sizeof(MessageHeader));

        receive_header(server_socket, buffer_header);

        t_list *respuesta_list = receive_package(server_socket, buffer_header);

        uint32_t rta = *((uint32_t*)list_get(respuesta_list, 0));
        return rta;
    }

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
