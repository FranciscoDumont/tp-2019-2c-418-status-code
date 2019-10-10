//
// Created by utnso on 01/10/19.
//

#include "protocolo.h"


/*
 *Funcion que te permite crear un paquete reservando la memoria que usa
 *  estructura:
 *          MessageHeader header:
 *                  MessageType type:
 *                  int data_size;
 *          void* stream; (Donde se encuentra la informacion)
 */

Paquete* crear_paquete(MessageType type)
{
    Paquete* paquete = malloc(sizeof(Paquete));
    paquete->header.type = type;
    paquete->header.data_size = 0;
    paquete->stream = NULL;

    return paquete;
}

/*
 * Funcion que permite agregarle informacion a un paquete
 */

void agregar_a_paquete(Paquete* paquete, void* valor, int tamanio)
{
    //Reserva mas memoria en stream para agregar informacion
    paquete->buffer->stream = realloc(paquete->stream, paquete->header.data_size + tamanio + sizeof(int));

    //La info se copia en la direccion de stream corrida tantos lugares como datos ya habia (Osea al final)
    //memcpy(DIR DEL FINAL, INFO A COPIAR, TAMANIO )
    memcpy(paquete->stream + paquete->header->size, &tamanio, sizeof(int));
    memcpy(paquete->stream + paquete->header->size + sizeof(int), valor, tamanio);

    //Cambio el tamanio del paquete
    paquete->header.data_size += tamanio + sizeof(int);
}

/*
 * Funcion que permite recibir un header
 */

int receive_header(int socket, Paquete* paquete) {
    int rec;
    rec = recv(socket, &paquete->header, sizeof(MessageHeader), MSG_WAITALL);
    /*
    if (rec > 0) {
        if(NETWORK_DEBUG_LEVEL >= NW_ALL_DISPLAY) {
            custom_print("[NETWORK_INFO][HEADER_RECIEVED_FROM_%d_(%d_bytes)]\n", source, rec);
        }
    } else {
        if(NETWORK_DEBUG_LEVEL >= NW_NETWORK_ERRORS) {
            custom_print("[NETWORK_ERROR][ERROR_RECIEVING_HEADER_FROM_%d]\n", source);
        }
    }*/
    return rec;
}

/*
 * Funcion que permite recibir el buffer de un mensaje
 */

int receive_data(int socket, Paquete* paquete, int data_size) {
    int rec;
    rec = recv(socket, paquete->stream, data_size, MSG_WAITALL);
    /*
    if (rec > 0) {
        if(NETWORK_DEBUG_LEVEL >= NW_ALL_DISPLAY) {
            custom_print("[NETWORK_INFO][DATA_RECIEVED_FROM_%d_(%d_bytes)]\n", source, rec);
        }
    } else {
        if(NETWORK_DEBUG_LEVEL >= NW_NETWORK_ERRORS) {
            custom_print("[NETWORK_ERROR][ERROR_RECIEVING_DATA_FROM_%d]\n", source);
        }
    }*/

    return rec;
}

/*
 * Funcion que permite convertir un buffer en una lista
 */

t_list* recibir_paquete(Paquete paquete)
{
    int desplazamiento = 0;
    void * buffer;
    t_list* valores = list_create();
    int tamanio;

    //Recibe el buffer entero en un puntero a void
    buffer = paquete.stream;

    //Pasa los datos de ese buffer a una lista
    while(desplazamiento < paquete.header.data_size)
    {   //el buffer esta dividido en tamaño dato
        memcpy(&tamanio, buffer + desplazamiento, sizeof(int));//Copia en tamanio el primer int del buffer
        desplazamiento += sizeof(int);//desplaza un int el valor de desplazamiento para la proxima iteracion
        char* valor = malloc(tamanio);//reserva la memoria para el valor
        memcpy(valor, buffer + desplazamiento, tamanio);//copa el valor
        desplazamiento += tamanio;//ddesplaza el tamanio del dato en el desplazamiento para la proxima iteracion
        list_add(valores, valor);//lo agrega a la lista
    }

    free(buffer);
    return valores; // devuelve al lista
    return NULL;
}

/*
 * Funcion que elimina la memoria reservada por el paquete
 * USAR SIEMPRE
 * SIEMPRE FORRO!!!!!!!!!!
 */
void eliminar_paquete(Paquete* paquete)
{
    free(paquete->stream);
    free(paquete->header);
    free(paquete);
}