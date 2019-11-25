//
// Created by utnso on 10/11/19.
//

#ifndef FUNCIONES_EMI_STRUCTS_H
#define FUNCIONES_EMI_STRUCTS_H

#include <stdint.h>
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
    int8_t estado; // 0:borrado 1:Ocupado 2:Directorio
    unsigned char nombre_archivo[NOMBRE_ARCHIV_MAX];
    int32_t ptr_bloque_padre;
    int32_t size;
    int64_t fecha_creacion;
    int64_t fecha_modificacion;
    int32_t GBloque[PUNTEROS_SIMPLES_CANTIDAD];
}GFile;

#endif //FUNCIONES_EMI_STRUCTS_H
