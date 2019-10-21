#ifndef SUSE_LIBRARY_H
#define SUSE_LIBRARY_H

#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <hilolay/alumnos.h>
#include <commons/config.h>
#include <altaLibreria/connections.h>
#include <altaLibreria/structures.h>
#include "libSuseStructures.h"

/**
 * Inicializo la conexion con el servidor
 */
void suse_init();

/**
 * Leo el archivo de configuracion y cargo sus valores en el la estructura de configuracion
 */
void read_config_options();

/**
 * Creo las estructuras correspondientes al hilo que me pasa por parametro hilolay en SUSE
 * @param tid
 * @return 0 en caso de exito, -1 en caso de error?
 */
int suse_create(int tid);

/**
 * Hilolay me pide que replanifique el proceso, le paso el siguiente hilo segun la planificacion de SUSE
 * @return el sgte hilo segun la planificacion de SUSE o -1 si no puedo?, TODO:hacer que se quede bloqueado en un recv hasta que algun close me libere lugar para el hilo
 */
int suse_schedule_next(void);

int suse_join(int tid);

/**
 * Da por finalizado el thread indicado en el parametro
 * @param tid, thread a cerrar
 * @return
 */
int suse_close(int tid);

int suse_wait(int tid, char *sem_name);

int suse_signal(int tid, char *sem_name);

/**
 * Por decision de diseño, todas las funciones de SUSE envian un codigo de rta, segun el codigo, realizo una u otra accion
 * @return Codigo de rta enviado por SUSE
 */
int confirm_action();

#endif //HILOLAY_LIBRARY_H