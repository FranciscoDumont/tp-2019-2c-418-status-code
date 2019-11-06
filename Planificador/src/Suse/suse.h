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
 * Inicializo los semaforos pasados por archivo de configuracion, los agrego a la lista de semaforos y tambien a la
 * lista de bloqueos.
 */
void initialize_semaphores();

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
 * Creo las estructuras necesarias para representar al tid suministrado por hilolay. En esta operacion hay varios casos
 * posibles:
 * * Grado de multiprogramacion >= limite:
 * * * Si el programa ya estaba en ejecucion(exec = true), se agrega el hilo a NEW y se retorna 1.
 * * * Si el programa no estaba en ejecucion, se agrega hilo a NEW y se retorna -1, de esta manera, el cliente se cuelga
 * * * * y se queda esperando una nueva confirmacion, ya que la funcion init, coloca al primer hilo en ejecucion(cliente)
 * * * * logrando una inconsistencia, estaria ejecutando en cliente pero en NEW en SUSE, si lo bloqueamos, logramos
 * * * * que no se produzca tal incosistencia.
 * * Grado de multiprogramacion < limite:
 * * * Si el programa ya estaba en ejecucion, se verifica si  el estado exec es distinto de NULL, en cuyo caso, el hilo
 * * * * se agregara a la lista de readys.
 * * * Si el programa no estaba en ejecucion, se agrega el hilo directo al estado de ejecucion del programa
 * @param fd
 * @param ip
 * @param port
 * @param received(tid)
 */
void suse_create(int fd, char* ip, int port, t_list* received);

/**
 * Retorna el proximo TID a ejecutar segun el algoritmo de planificacion de SUSE. En esta operacion hay varios casos
 * posibles:
 * * Lista de ready del programa > 0:
 * * * exec != NULL, se debe llamar al planificador para que me diga el proximo hilo a ejecutar, actualizar los
 * * * * intervalos de listo y de ejecucion, mover el proximo hilo a ejecutar a exec y al que estaba en exec a la lista
 * * * * de listos, retorna el proximo tid a ejecutar.
 * * * exec == NULL, se llama al planificador, se obtiene el proximo hilo a ejecutar, se actualizan los intervalos
 * * * * correspondientes y se coloca el hilo a ejecutar en exec, retorna el proximo tid a ejecutar.
 * * Lista de ready del programa = 0:
 * * * exec != NULL, se actualizan los intervalos de ejecucion, y se retorna el mismo hilo, ya que no hay otros
 * * * * disponibles, retorna el proximo tid a ejecutar.
 * * * exec == NULL, se coloca al programa en una lista de espera y se bloquea al cliente hasta que haya algun hilo
 * * * * disponible, retorna -1.
 * @param fd
 * @param ip
 * @param port
 * @param received
 * @note el valor de los tids es siempre >= 0, de ahi que el codigo de error sea -1.
 */
void suse_schedule_next(int fd, char * ip, int port, t_list* received);

/**
 * Realiza las tareas administrativas como actualizar los intervalos y mover los hilos de una cola a la otra luego de
 * replanificar el proceso
 * @param program
 * @return int tid a ejecutar a continuacion, o -1 si el programa no esta en ejecucion
 */
int schedule_next(t_program* program);

/**
 * Retorno el sgte hilo a ejecutarse utilizando el algoritmo SJF-E
 * @param program, programa al que le busco el nuevo hilo
 * @return t_thread*, sgte hilo a ejecutarse
 */
t_thread* schedule_new_thread(t_program* program);

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

/**
 * Cada vez que se ejecuta suse_close, se libera una posicion de las limitadas por la multiprogramacion, esta funcion
 * se utiliza para repartir hilos segun los siguientes criterios:
 * * Primero:me busca al primer hilo de la lista de NEW que pertenezca a algun programa en ejecucion, si hay, se
 * * * consulta si el programa pertenece a la lista de los programas que solicitaron un hilo, si lo es, lo debe agregar
 * * * al programa correspondiente y avisarle al cliente para que se desbloquee, si no lo es, deberia agregarlo
 * * * solamente a la lista de ready de su programa.
 * * Segundo:si no habia hilos pertenecientes a programas en ejecucion, se agrega el hilo al mismo y se habilita su
 * * * ejecucion, desbloqueando al cliente.
 */
void distribute_new_thread();

/**
 * Agrego un hilo a su programa correspondiente, inicializo los intervalos y le aviso al cliente correspondiente
 * @param program
 * @param thread
 * @param header
 */
void assign_thread(t_program* program, t_thread* thread, MessageType header);

/**
 * Genera una operación de wait sobre el semáforo dado, reduzco el valor del mismo, si es menor a 0, el hilo se agrega
 * a la lista de bloqueados.
 * @param fd
 * @param ip
 * @param port
 * @param received
 */
void suse_wait(int fd, char * ip, int port, t_list* received);

/**
 * Genera una operación de signal sobre el semáforo dado
 * @param fd
 * @param ip
 * @param port
 * @param received
 */
void suse_signal(int fd, char * ip, int port, t_list* received);

/**
 * Funcion que ejecuta a la funcion que produce las metricas, corre en un hilo paralelo
 * @param arg
 * @return Este return esta solo para cumplir con la firma de las funcion que acepta pthread, no devuelve nada
 */
void* metrics_function(void* arg);

/**
 * Funcion que genera y loggea las metricas
 * @return Este return esta solo para cumplir con la firma de las funcion que acepta pthread, no devuelve nada
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
 * Genero las metricas de los hilos en NEW de un programa dado
 * @param news
 * @return
 */
char* new_threads_metrics(t_list* news);

/**
 * Genero las metricas de los hilos en READY de un programa dado
 * @param program
 * @return
 */
char* ready_threads_metrics(t_program* program);

/**
 * Genero las metricas de los hilos en RUN de un programa dado
 * @param program
 * @return
 */
char* run_threads_metrics(t_program* program);

/**
 * Genero las metricas de los hilos en BLOCKED de un programa dado
 * @param program
 * @return
 */
char* blocked_threads_metrics(t_program* program);

/**
 * Genero las metricas de los hilos en EXIT de un programa dado
 * @param exited
 * @return
 */
char* exited_threads_metrics(t_list* exited);

/**
 * Genero las metricas del sistema
 * @return char* metricas
 */
char* generate_system_metrics();

/**
 * Genero las metricas para cada semaforo
 * @return
 */
char* generate_semaphore_metrics();

//--HELPERS

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
 * Retorno el hilo al que pertenece un tid y pid dados, ṕrimero busca en la lista de ready del programa, luego en la
 * de EXIT, luego en la de NEW, y finalmente(proximamente) en la de BLOCKED(TODO:verificar que esta ultima sea necesaria)
 * @param program
 * @param tid
 * @return t_thread*
 */
t_thread* find_thread(t_program* program, TID tid);

/**
 * Retorno el semaforo al que le corresponde un id dado
 * @param id, char*
 * @return t_semaphore*
 */
t_semaphore* find_semaphore(char* id);

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
 * Retorna una lista con los hilos en new de un programa dado, destruir despues de usar
 * @param program
 * @return
 */
t_list* threads_in_new_list(t_program* program);

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
 * Retorna la cantidad de hilos en join_block de un programa especifico
 * @param program
 * @return
 */
int threads_in_join_block(t_program* program);

/**
 * Retorno la lista de los hilos bloqueados por un join
 * @param program
 * @return
 */
t_list* threads_in_join_block_list(t_program* program);

/**
 * Retorna la cantidad de hilos en semaphore_block de un programa especifico
 * @param program
 * @return
 */
int threads_in_semaphore_block(t_program* program);

/**
 * Retorno la lista de los hilos bloqueados por un semaphore
 * @param program
 * @return
 */
t_list* threads_in_semaphore_block_list(t_program* program);

/**
 * Retorna la cantidad de hilos en el estado EXEC de un programa especifico
 * @param program
 * @return int, cant de hilos en EXEC
 */
int threads_in_exec(t_program* program);

/**
 * Retorna una lista con los hilos en EXIT de un programa dado
 * @return
 */
t_list* threads_in_exit_list(t_program* program);

/**
 * Destruyo el programa dado junto a todas sus estructuras asociadas
 * @param pid, pid del programa a destruir
 */
void destroy_program(PID pid);

/**
 * Destruyo todos los hilos de EXIT que pertenezcan a un programa dado
 * @param pid
 */
void destroy_exit_threads(PID pid);

/**
 * Verifico si el hilo esta muerto(en la cola de EXIT)
 * @param thread, hilo que voy a revisar
 * @return bool, true para muerto, false para vivo
 */
bool blocking_thread_is_dead(t_thread* thread);

/**
 * Libero los posibles join_blocks que pueda llegar a tener un hilo
 * @param thread, hilo del que voy a liberar los blocks
 * @param program
 */
void free_join_blocks(t_thread* thread, t_program* program);

/**
 * Verifico si un programa dado esta en la lista de asking_for_thread
 * @param program
 * @return
 */
bool is_in_asking_for_thread(t_program* program);

/**
 * Remuevo un programa de la lista de asking_for_thread
 */
void remove_from_asking_for_thread(t_program* program);

/**
 * Hallo el tiempo de ejecucion total(hasta el momento) para un programa dado
 * @param exits
 * @param readys
 * @param semaphores
 * @param joins
 * @param exec
 */
struct timespec* total_exec_time(t_list* news, t_list* exits, t_list* readys, t_list* semaphores, t_list* joins, t_thread* exec);

/**
 * Hallo el tiempo de ejecucion(desde la creacion) de una lista de hilos de un programa dado
 * @param list
 * @param elapsed_time
 */
void find_exec_time_on_list(t_list* list, struct timespec* elapsed_time);

/**
 * Hallo el tiempo de ejecucion(desde la creacion) para un hilo dado
 * @param thread
 * @param timestamp
 */
void find_exec_time(t_thread* thread, struct timespec* timestamp);

/**
 * Hallo el tiempo de ejecucion(en EXEC) de una lista de hilos de un programa dado
 * @param list
 * @param elapsed_time
 */
void find_runtime_on_list(t_list* list, struct timespec* elapsed_time);

/**
 * Hallo el tiempo de ejecucion(en EXEC) para la lista de ejecucion de un programa dado
 * @param exec_list
 * @param timestamp
 */
void find_runtime(t_list* exec_list, struct timespec* timestamp);

/**
 * Hallo la diferencia de tiempo entre dos timespecs
 * @param start
 * @param end
 * @param diff
 */
void time_diff(struct timespec* start, struct timespec* end, struct timespec* diff);

#endif //SUSE_SUSE_H