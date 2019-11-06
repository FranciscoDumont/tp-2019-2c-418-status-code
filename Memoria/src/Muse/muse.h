//
// Created by utnso on 05/10/19.
//
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <pthread.h>
#include <math.h>
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
    bool is_shared;
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
} heap_metadata;


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
segment_t* crear_segmento(bool is_shared);
page_t* crear_pagina(int frame, int presence_bit, int modified_bit, int use);

// Devuelve un puntero a donde termina la estructura
void* mp_escribir_metadata(void* espacio_libre, uint32_t tam, int esta_libre);

// Devuelve un puntero al primer espacio libre en MP que sea del tam suficiente
int mp_buscar_frame_libre();

process_t* buscar_proceso(int id_proceso);
char* mapa_memoria_to_string();
#endif //TP_2019_2C_418_STATUS_CODE_MUSE_H
