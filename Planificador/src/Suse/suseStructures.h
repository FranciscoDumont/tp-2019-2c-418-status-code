#ifndef SUSE_SUSESTRUCTURES_H
#define SUSE_SUSESTRUCTURES_H

typedef int TID;
typedef char* PID;

/**
 * Enum para diferenciar los tipos de block, join o semaphore
 */
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
 * estuvo en cola de listo, y tambien un identificador de tiempo de inicio
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
 * Estructura encargada de representar un semaforo
 * Posee un identificador(char*), un valor maximo y un valor actual, los primeros dos se obtienen por archivo de
 * configuracion, el ultimo se obtiene tambien por archivo de configuracion, pero se va modificando a medida que se
 * realizan waits y signals sobre el mismo, y por ultimo una lista de threads bloqueados
 */
typedef struct _t_semaphore{
    char* id;
    int max_value;
    int current_value;
    t_list* blocked_threads;
} t_semaphore;

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
 * Struct generico que sirve para relacionar al tipo de bloqueo con la estructura correspondiente, esta formado por una
 * instancia del enum correspondiente al blockeo(JOIN o SEMAPHORE) y un void* que apunta a la estructura responsable
 * de representar el bloqueo correspondiente(t_join_block o t_semaphore_block).
 */
typedef struct _t_block{
    BlockType block_type;
    void* block_structure;
} t_block;

/**
 * Struct que representa un bloqueo generado por un join, esta formado por un puntero que apunta al hilo blockeado y
 * una lista que contiene a los hilos que le solicitaron un join
 */
typedef struct _t_join_block{
    t_thread* blocked_thread;
    t_thread* blocking_thread;
} t_join_block;

/**
 * Struct que representa un bloqueo generado por un wait, esta compuesto por un puntero al semaforo. Esta estructura es
 * solamente un wrapper(para mantener cierta logica) ya que la lista de procesos bloqueados eesta en el semaforo en si
 * mismo.
 */
typedef struct _t_semaphore_block{
    t_semaphore* semaphore;
} t_semaphore_block;
#endif //SUSE_SUSESTRUCTURES_H