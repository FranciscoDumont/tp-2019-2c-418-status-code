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
#include <commons/log.h>
#include <commons/collections/list.h>
#include <memory.h>
#include "../structs.h"
#include <time.h>

/*****************************
 **** Funciones auxiliares****
 ****************************/

int obtenerBitMapSize(char* particion);

t_bitarray* obtenerBitMap(char* particion, GBloque* disco);

t_list * buscarBloquesMemoriaLibres(int cantidad,GBloque* disco, char* nombreParticion);

char* obtenerFechaActual();

int obtenerTamanioArchivo (char* file);

int obtenerNBloquesBitMap(double disco_size);

int obtenerCantidadBytsBitmap(int disco_size);

void inicializarBloqueDirectorio(GBloque* bloque);

int buscarPath(t_list* pathDividido);

t_list* hallar_posibles_nodos(char* nombreNodo);

t_list* dividirPath(char* path);

GBloque* mapParticion (char* particion);

void munmapParticion (GBloque* disco, char* nombreParticion);

int obtenerNodoLibre (GFile* comienzoTabla);

char* crearBitMap(int bitmap_count_bloques);

/*****************************
 **** Funciones formateo *****
 ****************************/

void escribirHeader(GBloque* puntero_disco, int bitmap_size);

void escribirBitMap(GBloque* puntero_disco, int  bitmap_bloques_count, int bitmap_size);

void escribirNodeTabla (GBloque* puntero_disco);

int formatear (char* nombre_particion, t_log* logger);

/*****************************
 **** Funciones Mostrar ******
 ****************************/

void mostrarHeader(GBloque* disco );

void mostrarBitMap(GBloque* disco, int bitmap_count_bloques);

void mostrarTablaNodos(GBloque* disco);

void mostrarParticion(char* nombre_particion);

void mostrarNodo(GFile* nodo,GBloque* disco);

#endif //SERVIDOR_FILESYSTEM_H
