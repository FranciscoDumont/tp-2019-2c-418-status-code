//
// Created by utnso on 24/10/19.
//

#ifndef SERVIDOR_FILESYSTEM_H
#define SERVIDOR_FILESYSTEM_H

#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <commons/bitarray.h>
#include <memory.h>


#define  IDENTIFICADOR_TAMANIO 3
#define  NOMBRE_ARCHIV_MAX 71
#define  PUNTEROS_SIMPLES_CANTIDAD 1000
#define  RELLENO_TAMANIO 4081
#define BLOQUE_TAMANIO 4096
#define CANTIDAD_ARCHIVOS_MAX 1024


typedef struct bloque_t{
    unsigned char bytes[BLOQUE_TAMANIO];
} GBloque;

typedef struct header_t{
    unsigned char identificador[IDENTIFICADOR_TAMANIO];
    int32_t version;
    int32_t bitmap_inicio;
    int32_t bitMap_tamanio;
    unsigned char relleno[RELLENO_TAMANIO];
}GHeader;

typedef struct file_t{
    uint8_t estado; // 0:borrado 1:Ocupado 2:Directorio
    unsigned char nombre_archvio[NOMBRE_ARCHIV_MAX];
    int32_t ptr_bloque_padre;
    int32_t size;
    int64_t fecha_creacion;
    int64_t fecha_modificacion;
    int32_t GBloque[PUNTEROS_SIMPLES_CANTIDAD];
}GFile;

int obtenerNBloquesBitMap(int disco_size);

void escribirHeader(GBloque* puntero_disco, int bitmap_size);

char* crearBitMap(int bitmap_count_bloques);

void escribirBitMap(GBloque* puntero_disco, int  bitmap_count);

void escribirNodeTabla (GBloque* puntero_disco);

int formatear (char* nombre_particion);

void mostrarHeader(GBloque* disco );

void mostrarBitMap(GBloque* disco, int bitmap_count_bloques);

void mostrarTablaNodos(GBloque* disco);

void mostrarParticion(char* nombre_particion);

#endif //SERVIDOR_FILESYSTEM_H
