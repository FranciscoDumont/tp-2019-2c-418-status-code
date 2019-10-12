#ifndef SUSE_SUSE_H
#define SUSE_SUSE_H

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>

#include <commons/config.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/collections/dictionary.h>
#include <altaLibreria/connections.h>
#include <altaLibreria/structures.h>
#include "suseStructures.h"

//--Inicializo el log
void start_log();

//--Inicializo las listas de estados
void initialize_structures();

//--Leo las configuraciones del archivo y las cargo en el struct SUSEConfig
void read_config_options();

//--Funcion encargada de definir las tres funciones para el servidor
void server_function();

//--Creo un nuevo proceso y lo cargo en la lista de procesos
void create_new_program(char* ip, int port);

//--Esta función es invocada cuando se necesita crear un nuevo hilo, donde la función que
//se pase por parámetro actuará como main del mismo, finalizando el hilo al terminar esa función
//--Recibe TID
//--Retorna int, mismo TID?
void suse_create(int fd, char* ip, int port, t_list* cosas);

//--Obtiene el próximo hilo a ejecutar
//--Recibe nada
//--Retorna el proximo TID a ejecutar
void* suse_schedule_next(void* newComm);

//--Genera una operación de wait sobre el semáforo dado
void* suse_wait(void* newComm);

//--Genera una operación de signal sobre el semáforo dado
void* suse_signal(void* newComm);

//--Bloquea al thread esperando que el mismo termine. El thread actual pasará a estar
//BLOCKED y saldrá del mismo luego de que el PID indicado finalice su ejecución. También es posible
//realizar un join a un thread ya finalizado
//--Recibe TID
//--Devuelve int, mismo TID?
void* suse_join(void* newComm);

//--Da por finalizado al TID indicado. El thread actual pasará a estar EXIT.
void* suse_return(void* newComm);

//--Funcion que ejecuta la funcion encargada de generar las metricas cada cierto tiempo
void* metrics_function(void* arg);

//--Funcion que genera y loggea las metricas
void generate_metrics();

//--Genero las metricas de cada hilo
char* generate_thread_metrics();

//--Genero las metricas de cada programa
char* generate_program_metrics();

//--Genero las metricas del sistema
char* generate_system_metrics();

//--HELPERS

//--Genero un identificador de proceso en base a la ip y al puerto desde los que se conecta el cliente
char* generate_pid(char* ip, int port);

//--Hallo el grado de multiprogramacion total del sistema(cant de hilos que no estan en new)
int multiprogramming_grade();

struct timespec get_time();

void free_list(t_list* received, void(*element_destroyer)(void*));

#endif //SUSE_SUSE_H