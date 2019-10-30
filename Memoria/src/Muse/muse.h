//
// Created by utnso on 05/10/19.
//
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <altaLibreria/connections.h>
#include <altaLibreria/structures.h>


#ifndef TP_2019_2C_418_STATUS_CODE_MUSE_H
#define TP_2019_2C_418_STATUS_CODE_MUSE_H


t_log * logger;

t_list* process_table;

void* main_memory;

typedef struct {
    int pid;
    t_list* segments;
} process_t;

typedef struct {
    int is_shared;
    t_list* pages;
} segment_t;

typedef struct {
    int presence_bit; // Para saber si esta en memoria principal
    int modified_bit; // Para el algoritmo de reemplazo
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
uint32_t muse_alloc(uint32_t tam);
void muse_free(uint32_t dir);
int muse_get(void *dst, uint32_t src, size_t n);
int muse_cpy(uint32_t dst, void *src, int n);
uint32_t muse_map(char *path, size_t length, int flags);
int muse_sync(uint32_t addr, size_t len);
int muse_unmap(uint32_t dir);

process_t* crear_proceso(int id);
segment_t* crear_segmento(int is_shared);
page_t* crear_pagina(int presence_bit, int modified_bit);


#endif //TP_2019_2C_418_STATUS_CODE_MUSE_H
