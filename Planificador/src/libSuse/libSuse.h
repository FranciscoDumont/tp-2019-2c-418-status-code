#ifndef SUSE_LIBRARY_H
#define SUSE_LIBRARY_H

#include <stdio.h>
#include <unistd.h>

#include <hilolay/alumnos.h>
#include <commons/config.h>
#include <altaLibreria/connections.h>
#include <altaLibreria/structures.h>
#include "libSuseStructures.h"

//--Inicializo la conexion con el servidor
void suse_init();

//--Leo el archivo de configuracion y cargo sus valores en el la estructura de configuracion
void read_config_options();

//--Creo un hilo en SUSE, si la conexion con el servidor no fue realizada, realizo esta primero
int suse_create(int tid);

int suse_schedule_next(void);

int suse_join(int tid);

int suse_close(int tid);

int suse_wait(int tid);

int suse_signal(int tid);

int suse_return(int tid);

#endif //HILOLAY_LIBRARY_H