//
// Created by utnso on 24/10/19.
//

#include "fileSystem.h"
#define  IDENTIFICADOR_TAMANIO 3
#define  NOMBRE_ARCHIV_MAX 71
#define  PUNTEROS_SIMPLES_CANTIDAD 1000
#define  RELLENO_TAMANIO 4081

#define BLOQUE_TAMANIO 4096
#define CANTIDAD_ARCHIVOS_MAX

// LOS BLOQUES USADOS STIENEN QUE ESTAR MARCADOS COMO USADOS



//Posible problema puede ser un int de 64 tenerlo en cuenta
//Tiene que tener un tamanio de 4096 bytes
typedef struct bloque{
    unsigned char bytes[BLOQUE_TAMANIO]
}GBloque;

typedef struct header_t{
    unsigned char identificador[IDENTIFICADOR_TAMANIO];
    int32_t version;
    int32_t bitmap_inicio;
    int32_t bitMap_tamanio;
    unsigned char relleno[RELLENO_TAMANIO];
}GHeader;

//Estado es de tipo char porque dice que pesa 1 byte
//ptrGBloque el numreo de bloque donde esta almacenado el directorio padre
typedef struct file_t{
    uint8_t estado; // 0:borrado 1:Ocupado 2:Directorio
    unsigned char nombre_archvio[NOMBRE_ARCHIV_MAX];
    int32_t ptr_bloque_padre;
    int32_t fecha_creacion;
    int32_t fecha_modificacion;
    int32_t GBloque[PUNTEROS_SIMPLES_CANTIDAD];
}GFile;

int formatear (){

    return 0;
}

void escribirHeader(){

}

void escribirBitMap(){

}

void escribirNodeTabla (){

}

int obtenerTamanioArchivo (char* file ){
    File* fd = fopen(file,"r");

    fseek(fd, 0L, SEEK_END);
    int32_t dfSize = ftell(fd);

    fclose(fd);
    return dfSize;
}

