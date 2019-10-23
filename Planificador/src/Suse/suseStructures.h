#ifndef SUSE_SUSESTRUCTURES_H
#define SUSE_SUSESTRUCTURES_H

typedef int TID;
typedef char* PID;

typedef enum _BlockType {
    JOIN,
    SEMAPHORE
} BlockType;

/**
 * Estructura encargada de almacenar las configuraciones que se pasan por archivo
 */
typedef struct _SUSEConfig {
    int listen_port;
    int metrics_timer;
    int max_multiprog;
    char** sem_ids;
    char** sem_init;
    char** sem_max;
    double alpha_sjf;
} SUSEConfig;

/**
 * Estructura encargada de representar un hilo de un programa
 * Posee basicamente un identificador propio, un identificador del programa al que pertenece
 * y dos listas, en la primera se almacenan los intervalos de ejecucion, y en la segunda los intervalos en los que
 * estuvo en cola de listo
 */
typedef struct _t_thread{
    TID tid;
    PID pid;
    t_list* exec_list;
    t_list* ready_list;
    struct timespec* start_time;
} t_thread;

/**
 * Estructura encargada de representar un programa
 * Posee un identificador de programa formado de la sgte manera: IP::PORT, el socket del cliente que lo origino
 * ademas de una lista de hilos en estado de listo
 * y un hilo en estado de ejecucion, finalmente un campo booleano indicando si alguno de sus hilos se comenzo a ejecutar
 */
typedef struct _t_program{
    PID pid;
    int fd;
    t_list* ready;
    t_thread* exec;
    bool executing;
} t_program;

/**
 * Estrucura encargada de representar una respuesta al cliente
 * Esta formada por el socket cliente al cual responder, un int que va a representar la respuesta y el cliente
 * se encargara de interpretar y un Header de respuesta
 */
typedef struct _t_new_response{
    int fd;
    int response;
    MessageType header;
} t_new_response;

/**
 * Estructura encargada de representar un intervalo de tiempo
 * Esta formada a su vez por otras dos estructuras timespec, las cuales almacenan timestamps y con las cuales se puede
 * operar para obtener la diferencia entre ambas
 */
typedef struct _t_interval{
    struct timespec* start_time;
    struct timespec* end_time;
} t_interval;

/**
 * Struct generico que sirve para relacionar al tipo de bloqueo con la estructura correspondiente
 */
typedef struct _t_block{
    BlockType block_type;
    void* block_structure;
} t_block;

/**
 * Struct que representa un bloqueo generado por un join
 */
typedef struct _t_join_block{
    t_thread* blocked_thread;
    t_list* blocking_threads;
} t_join_block;

/**
 * Struct que representa un bloqueo generado por un wait
 */
typedef struct _t_semaphore_block{
    int a;
} t_semaphore_block;
#endif //SUSE_SUSESTRUCTURES_H