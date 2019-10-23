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

/**
 * Inicializo el log
 */
void start_log();

/**
 * Inicializo semaforos, listas de estados comunes y la estructura de configuracion
 */
void initialize_structures();

/**
 * Leo las configuraciones del archivo y las cargo en el struct SUSEConfig
 */
void read_config_options();

/**
 * Funcion encargada de definir las tres funciones para el servidor(new, incoming y lost)
 */
void server_function();

/**
 * Creo un nuevo proceso y lo cargo en la lista de procesos
 * @param ip
 * @param port
 * @param fd socket del cliente
 */
void create_new_program(char* ip, int port, int fd);

/**
 * Creo las estructuras necesarias para representar al tid suministrado por hilolay
 * @param fd
 * @param ip
 * @param port
 * @param received(tid)
 * Retorna por socket un 0 en caso de exito o un -1 en caso de exito
 */
void suse_create(int fd, char* ip, int port, t_list* received);

/**
 * Retorna el proximo TID a ejecutar segun el algoritmo de planificacion de SUSE
 * @param fd
 * @param ip
 * @param port
 * @param received
 * Retorno por socket el proximo tid a ejecutar
 */
void suse_schedule_next(int fd, char * ip, int port, t_list* received);

/**
 * Nucleo de la planificacion, actualizo las listas de intervalos de ready y exec de los
 * threads sobre los que trabajo.
 * @param program
 * @return int tid a ejecutar a continuacion, o -1 si el programa no esta en ejecucion
 */
int schedule_next(t_program* program);

//--Bloquea al thread esperando que el mismo termine. El thread actual pasará a estar
//BLOCKED y saldrá del mismo luego de que el PID indicado finalice su ejecución. También es posible
//realizar un join a un thread ya finalizado
//--Recibe TID
//--Devuelve int, mismo TID?
/**
 * Bloqueo un thread esperando que el mismo termine. El thread actual pasara a estar en BLOCKED y saldra solo cuando el
 * hilo que lo bloqueo termine su ejecucion con suse_close. Es posible bloquear a un hilo con un hilo ya finalizado.
 * @param fd
 * @param ip
 * @param port
 * @param received
 */
void suse_join(int fd, char * ip, int port, t_list* received);

/**
 * Da por finalizado el TID indicado, el thread actual pasa a EXIT, llamo a la funcion de las
 * metricas y elimino la estructura del programa
 * @param fd
 * @param ip
 * @param port
 * @param received
 */
void suse_close(int fd, char * ip, int port, t_list* received);

//--Genera una operación de wait sobre el semáforo dado
void* suse_wait(void* newComm);

//--Genera una operación de signal sobre el semáforo dado
void* suse_signal(void* newComm);

/**
 * Funcion que ejecuta a la funcion que produce las metricas, corre en un hilo paralelo
 * @param arg
 * @return
 */
void* metrics_function(void* arg);

/**
 * Funcion que genera y loggea las metricas
 */
void* generate_metrics(void* arg);

/**
 * Genero las metricas de cada hilo
 * @return char* metricas
 */
char* generate_thread_metrics();

/**
 * Genero las metricas de cada programa
 * @return char* metricas
 */
char* generate_program_metrics();

/**
 * Genero las metricas del sistema
 * @return char* metricas
 */
char* generate_system_metrics();

//--HELPERS

/**
 * Genero un identificador de proceso en base a la ip y al puerto desde los que se conecta el cliente
 * @param ip
 * @param port
 * @return PID pid
 */
PID generate_pid(char* ip, int port);

/**
 * Hallo el grado de multiprogramacion total del sistema(cant de hilos que no estan en new)
 * @return int grado
 */
int multiprogramming_grade();

/**
 * Retorno una estructura que representa al tiempo en segundos y microsegundos
 * @return
 */
struct timespec get_time();

/**
 * Wrapper para liberar una lista, nombre mas corto
 * @param received Lista
 * @param element_destroyer
 */
void free_list(t_list* received, void(*element_destroyer)(void*));

/**
 * Retorno el programa al que le corresponde un PID dado
 * @param pid
 * @return t_program*
 */
t_program* find_program(PID pid);

/**
 * Creo un hilo para responderle al cliente
 * @param fd
 * @param response
 * @param header
 */
void create_response_thread(int fd, int response, MessageType header);

/**
 * Creo un paquete de respuesta con los datos dados(para enviar al cliente)
 * @param fd
 * @param response
 * @param header
 * @return
 */
void* create_response_package(int fd, int response, MessageType header);

/**
 * Funcion encargada de enviar la respuesta al cliente
 * @param response_package
 * @return
 */
void* response_function(void* response_package);

/**
 * Hallo el ultimo intervalo de la lista de ejecucion
 * @param thread
 * @return interval* interval
 */
t_interval* last_exec(t_thread* thread);

/**
 * Hallo el ultimo intervalo de la lista de listos
 * @param thread
 * @return interval* interval
 */
t_interval* last_ready(t_thread* thread);

/**
 * Creo un nuevo intervalo con su memoria ya alocada
 * @return interval* interval
 */
t_interval* new_interval();

/**
 * Destruyo el hilo a cerrar
 * @param thread, hilo a destruir
 */
void destroy_thread(t_thread* thread);

/**
 * Verifico si un programa dado no posee mas hilos en ready, blocked y new
 * @param program, programa sobre el que se verifica
 * @return true si no tiene mas hilos, false si tiene mas hilos
 */
bool no_more_threads(t_program* program);

/**
 * Retorna la cantidad de hilos en el estado NEW de un programa especifico
 * @param program
 * @return int, cant de hilos en NEW
 */
int threads_in_new(t_program* program);

/**
 * Retorna la cantidad de hilos en el estado READY de un programa especifico
 * @param program
 * @return int, cant de hilos en READY
 */
int threads_in_ready(t_program* program);

/**
 * Retorna la cantidad de hilos en el estado BLOCKED de un programa especifico
 * @param program
 * @return int, cant de hilos en BLOCKED
 */
int threads_in_blocked(t_program* program);

/**
 * Retorna la cantidad de hilos en el estado EXEC de un programa especifico
 * @param program
 * @return int, cant de hilos en EXEC
 */
int threads_in_exec(t_program* program);

/**
 * Destruyo el programa dado junto a todas sus estructuras asociadas
 * @param program, programa a destruir
 */
void destroy_program(t_program* program);

#endif //SUSE_SUSE_H