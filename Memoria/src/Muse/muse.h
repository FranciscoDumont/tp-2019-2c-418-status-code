//
// Created by utnso on 05/10/19.
//
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <pthread.h>
#include <math.h>
#include <signal.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <altaLibreria/connections.h>
#include <altaLibreria/structures.h>


#ifndef TP_2019_2C_418_STATUS_CODE_MUSE_H
#define TP_2019_2C_418_STATUS_CODE_MUSE_H


// Variables Globales
t_log * logger;
void* MAIN_MEMORY;
t_list* PROCESS_TABLE;
int CANTIDAD_PAGINAS_ACTUALES;
int LIMITE_PAGINAS;
int MAPA_MEMORIA_SIZE;
t_bitarray* MAPA_MEMORIA;

typedef struct {
    int pid;
    t_list* segments;
} process_t;

typedef struct {
    int base;
    int size;
    t_list* pages;
} segment_t;

typedef struct {
    int frame;
    bool presence_bit; // Para saber si esta en memoria principal
    bool modified_bit; // Para el algoritmo de reemplazo
    bool use_bit;
} page_t;


typedef struct {
    uint32_t size;
    bool isFree;
} __attribute__((packed))
heap_metadata;


// Configuraciones
typedef struct MEMConfig{
    int listen_port;
    int memory_size;
    int page_size;
    int swap_size;
} MEMConfig;

t_config * config_file;
MEMConfig config;

// Funcion encargada de definir las tres funciones para el servidor
void* server_function(void* arg);

void tests_memoria();

// Lee el archivo de configuracion y lo carga en una estructura
void read_memory_config();

int muse_init(int id, char *ip, int puerto);
void muse_close();
uint32_t muse_alloc(uint32_t tam, int id_proceso);
void muse_free(uint32_t dir);
int muse_get(void *dst, uint32_t src, size_t n);
int muse_cpy(uint32_t dst, void *src, int n);
uint32_t muse_map(char *path, size_t length, int flags);
int muse_sync(uint32_t addr, size_t len);
int muse_unmap(uint32_t dir);

process_t* crear_proceso(int id);

/**
 * Constructor de segmento, se utiliza cuando no hay espacio en otro segmento o no se pueden extender por tener a otro
 * contiguo
 * @param process_t* process, proceso al que le voy a agregar el segmento
 * @param int paginas_necesarias, cant de paginas necesarias para el segmento
 * @return
 */
segment_t* crear_segmento(proceso_t* process, int paginas_necesarias);

/**
 * Busco frames libres y se los asigno a las paginas que necesita el segmento
 * @param int paginas_necesarias, cant de paginas necesarias
 * @param segment_t* nuevo_segmento, asigno los frames a las paginas que necesita el segmento
 */
void asignar_paginas(int paginas_necesarias, segment_t * nuevo_segmento);

/**
 * Constructor de las paginas
 * @param frame
 * @param presence_bit
 * @param modified_bit
 * @param use_bit
 * @return
 */
page_t* crear_pagina(int frame, int presence_bit, int modified_bit, int use_bit);

/**
 * Asigno la primera metadata de uso para un segmento nuevo
 * @param segment_t* segment, segmento al que le voy a agregar la primera metadata
 * @param int tam, tamaño de la memoria reservada posterior a la metadata
 */
void asignar_primer_metadata(segment_t* segment, int tam);

/**
 * Asigno la ultima metadata a un segmento para indicar el espacio libre
 * @param segment
 * @param tam
 * @param paginas_necesarias
 */
void asignar_ultima_metadata(segment_t* segment, int tam, int paginas_necesarias);

/**
 * Escribo la metadata correspondiente en una direccion de memoria dada
 * @param void* espacio_libre, puntero sobre el que voy a escribir la metadata
 * @param int tam, tamaño libre u ocupado a escribir en la metadata
 * @param int esta_libre, booleano que me indica si el tamaño indicado es de espacio libre o de espacio reservado
 * TODO: verificar si habria que sumarle 1 a la direccion retornada
 * @return retorno la direccion posterior a la escritura(final de la metadata)
 */
void* mp_escribir_metadata(void* espacio_libre, uint32_t tam, int esta_libre);

/**
 * Itero el bitarray buscando un frame libre
 * @return numero de frame de mem ppal
 */
int mp_buscar_frame_libre();

/**
 * Busco la cantidad de frames libres para verificar si puedo allocar en MP
 * @return
 */
int cant_frames_libres();

/**
 * Busco un proceso por id(socket cliente)
 * @param int id_proceso
 * @return retorno el proceso correspondiente al id dado
 */
process_t* buscar_proceso(int id_proceso);

char* mapa_memoria_to_string();

/**
 * Traduzco la direccion virtual de un segmento dado en la direccion "fisica" dentro de mi memoria
 * @param un_segmento
 * @param direccion_virtual
 * @return direccion "fisica"
 */
void* traducir_virtual(segment_t* un_segmento, uint32_t direccion_virtual);

/**
 * Busco si el segmento tiene algun espacio lo suficientemente grande como para almacenar el nuevo alloc
 * @param segment_t* segmento, segmento q voy a revisar
 * @param uint32_t* puntero, aca voy a asignar la direccion virtual que apunta al md que me indica el espacio libre
 * @param uint32_t tam, tamaño de la memoria a allocar(en realidad es esto mas el tamaño que ocupa un md)
 * @param uint32_t espacio_libre, me indica la cantidad de bytes que habia libres antes de reservar la nueva memoria en este espacio
 * @return true si hay espacio, false si no lo hay
 */
bool tiene_espacio_libre(segment_t* segmento, uint32_t * puntero, uint32_t tam, uint32_t tam);

/**
 * Hallo el espacio libre que posee un segmento, solo contando el ultimo md
 * @param segment_t* segmento, segmento al que le averiguo el espacio libre
 * @param uint32_t* dir_virtual_md_libre_temp, direccion virtual que apunta al ultimo md
 * @return retorno el espacio libre al final
 */
uint32_t hallar_espacio_libre(segment_t* segmento, uint32_t * dir_virtual_md_libre_temp);

/**
 * Retorno la direccion "fisica" al primer metadata de libre dentro de un segmento
 * @param un_segmento
 * @return
 */
void* puntero_a_mp_del_primer_metadata_libre(segment_t* un_segmento);

/**
 * Retorno la cantidad de memoria OCUPADA de un segmento
 * @param segment_t* un_segmento, segmento sobre el que verifico la memoria ocupada
 * @return retorno la cantidad de memoria ocupada del segmento en bytes
 */
int segmento_ocupado_size(segment_t* un_segmento);


#endif //TP_2019_2C_418_STATUS_CODE_MUSE_H
