/*
 * Explicacion de que hace mmap
 * mmap(NULL , int tamanio_de_lo_que_mapeas , int proteccion , int flags , int puntero_a_archivo , off_t tamanio de las paginas )
 * Proteccion es un enum
 * Puntero_a_archivo usar open();
 *
 *
 */

//
// Created by utnso on 24/10/19.
//

#include "fileSystem.h"
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>

#define  IDENTIFICADOR_TAMANIO 3
#define  NOMBRE_ARCHIV_MAX 71
#define  PUNTEROS_SIMPLES_CANTIDAD 1000
#define  RELLENO_TAMANIO 4081

#define BLOQUE_TAMANIO 4096
#define CANTIDAD_ARCHIVOS_MAX

// LOS BLOQUES USADOS STIENEN QUE ESTAR MARCADOS COMO USADOS

//Posible problema puede ser un int de 64 tenerlo en cuenta
//Tiene que tener un tamanio de 4096 bytes
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


/**
 * Permite saber el tamanio de un archivo. Copi-past de google
 * @param el path del archivo
 * @return el tamanio en byts
 */
int obtenerTamanioArchivo (char* file ){
    File* fd = fopen(file,"r");

    fseek(fd, 0L, SEEK_END);
    int32_t dfSize = ftell(fd);

    fclose(fd);
    return dfSize;
}


/**
 * Se encarga de dividir el bitmap en bloques
 * @param El bitmap
 * @param La cantidad de bloques que ocupa el bitmap
 * @return Retorna un array de bloques
 */

int bitMapToBloques(bitmap,int cantidad_bloques){
    //desarrollar
    reutrn
}

/**
 * Permite formatear un file system en un archibo bin
 * @param el path del archivo
 * @return
 */

void escribirHeader(GBloque* puntero_disco, int bitmap_size){
    GHeader* newHeader = (GHeader *) puntero_disco;
    memccpy(newHeader->identificador, "SAC",IDENTIFICADOR_TAMANIO);
    newHeader->version = 1;
    newHeader->bitmap_inicio = 1;
    newHeader->bitMap_tamanio = bitmap_size;
}

void escribirBitMap(GBloque* puntero_disco){

}
/**
 * Te permite escribir la tabla de nodos cuando formateas el disco
 * @param puntero_disco
 */
void escribirNodeTabla (GBloque* puntero_disco){
    GFile* nodo = (GFile *) puntero_disco;

    for(int numero_archivo = 0; numero_archivo < CANTIDAD_ARCHIVOS_MAX; numero_archivo++){
        nodo[numero_archivo].estado = 0;
    }
}

/**
 * Se encarga de dividir el bitmap en bloques
 * @param El bitmap
 * @param La cantidad de bloques que ocupa el bitmap
 * @return Retorna un array de bloques
 */

int bitMapToBloques(bitmap,int cantidad_bloques){
    //desarrollar
    reutrn
}
int formatear (char* nombre_archivo){
    int disco_size = obtenerTamanioArchivo(nombre_archivo);
    int bitmap_size = disco_size / BLOQUE_TAMANIO / 8;
    int disco_file = open(nombre_archivo, O_RDWR, 0);
    GBloque* disco = mmap(NULL, disco_size, PROT_READ|PROT_WRITE, MAP_FILE|MAP_SHARED,disco_file, BLOQUE_TAMANIO);

    printf("Se escribio el disco en:%d", disco);
    escribirHeader(disco,bitmap_size);

    printf("Se escribio el disco en:%d", disco + 1);
    escribirBitMap(disco +1);


    printf("Se escribio el disco en:%d", disco + 1 + bitmap_size);
    escribirBitMap(disco +1 + bitmap_size);


    return 0;
}

int main(){
    formatear("prueba.bin");
    return 0
}