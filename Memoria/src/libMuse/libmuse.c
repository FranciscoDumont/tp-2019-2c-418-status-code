#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <commons/memory.h>
#include <libmuse.h>


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
int muse_init(int id, char* ip, int puerto){
    // Inicializo variables
    log_create("libMuse.log", "LibMuse", 1, LOG_LEVEL_TRACE);
    config_file = config_create("config");

    if (!config_file){
        log_error(logger, "No se encontró el archivo de configuración");
        return -1;
    }
    // Levanto la configuracion
    config.ip = config_get_string_value(config_file, "IP");
    config.talking_port = config_get_int_value(config_file, "TALKING_PORT");

    // Creo socket
    if((server_socket = create_socket()) == -1) {
        log_error(logger, "Error al crear el socket\n");
        return -1;
    }

    // Conecto sockets
    if(connect_socket(server_socket, config.ip, config.talking_port) == -1){
        log_error(logger, "Error al conectarse al servidor\n");
        return -1;
    }

    //Procedo con esa tal comunicacion con el server
    t_paquete *package = create_package(MUSE_INIT);
    void* _id = malloc(sizeof(int));
    *((int*)_id) = id;
    add_to_package(package, _id, sizeof(int));

    if(send_package(package, server_socket) == -1){
        log_error(logger, "Error al enviar paquete\n");
        return -1;
    } else {
        // Espero la respuesta del server diciendo que todo está ok
        MessageHeader* buffer_header = malloc(sizeof(MessageHeader));

        receive_header(server_socket, buffer_header);

        t_list *cosas = receive_package(server_socket, buffer_header);

        int rta = *((int*)list_get(cosas, 0));
        return rta;
    }

}


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
