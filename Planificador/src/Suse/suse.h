#ifndef SUSE_H
#define SUSE_H

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#include <commons/config.h>
#include <commons/log.h>
#include <altaLibreria/connections.h>
#include <altaLibreria/structures.h>
#include "suseStructures.h"

//--Inicializo el log
void start_log();

//--Leo las configuraciones del archivo y las cargo en el struct SUSEConfig
void read_config_options();

//--Funcion encargada de definir las tres funciones para el servidor
void* server_function(void* arg);

//--Funcion que ejecuta la funcion encargada de generar las metricas cada cierto tiempo
void* metrics_function(void* arg);

//--Funcion que genera las metricas
void generate_metrics();

//--Inicializa recursos de biblioteca?
void suse_init(int fd, char * ip, int port, MessageHeader * headerStruct);

//--Esta función es invocada cuando se necesita crear un nuevo hilo, donde la función que
//se pase por parámetro actuará como main del mismo, finalizando el hilo al terminar esa función
void suse_create(int fd, char * ip, int port, MessageHeader * headerStruct);

//--Obtiene el próximo hilo a ejecutar
void suse_schedule_next(int fd, char * ip, int port, MessageHeader * headerStruct);

//--Genera una operación de wait sobre el semáforo dado
void suse_wait(int fd, char * ip, int port, MessageHeader * headerStruct);

//--Genera una operación de signal sobre el semáforo dado
void suse_signal(int fd, char * ip, int port, MessageHeader * headerStruct);

//--Bloquea al thread esperando que el mismo termine. El thread actual pasará a estar
//BLOCKED y saldrá del mismo luego de que el PID indicado finalice su ejecución. También es posible
//realizar un join a un thread ya finalizado
void suse_join(int fd, char * ip, int port, MessageHeader * headerStruct);

#endif //SUSE_H
