#ifndef SUSE_LIBRARY_H
#define SUSE_LIBRARY_H

#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <hilolay/alumnos.h>
#include <hilolay/hilolay.h>
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
 * @return 0 en caso de exito, -1 en caso de error
 */
int suse_create(int tid);

/**
 * Hilolay me pide que replanifique el proceso, le paso el siguiente hilo segun la planificacion de SUSE
 * @return el sgte hilo segun la planificacion de SUSE o -1 si sucede algun error
 */
int suse_schedule_next(void);

/**
 * Joinea dos hilos, el hilo en ejecucion es bloqueado
 * @param tid, hilo a joinear con el hilo en ejecucion
 * @return 0 en caso de exito o -1 en caso de error
 */
int suse_join(int tid);

/**
 * Da por finalizado el thread indicado en el parametro, libera todas las estructuras en el servidor, y los bloqueos
 * por join
 * @param tid, thread a cerrar
 * @return 0 en caso de exito o -1 en caso de error
 */
int suse_close(int tid);

/**
 * Realizo un wait en un semaforo, si el valor del mismo no alcanza, se manda el hilo a blocked
 * @param tid
 * @param sem_name
 * @return 0 en caso de exito o -1 en caso de error
 */
int suse_wait(int tid, char *sem_name);

/**
 * Realizo un signal sobre un semaforo
 * @param tid
 * @param sem_name
 * @return 0 en caso de exito o -1 en caso de error
 */
int suse_signal(int tid, char *sem_name);

/**
 * Por decision de diseño, todas las funciones de SUSE envian un codigo de rta, segun el codigo, realizo una u otra accion
 * @return Codigo de rta enviado por SUSE
 */
int confirm_action();

#endif //HILOLAY_LIBRARY_H