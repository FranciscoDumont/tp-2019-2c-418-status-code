#include "suse.h"

SUSEConfig* config;
t_log* logger;
t_list* NEW;
t_list* BLOCKED;
t_list* EXIT;
t_list* programs;
pthread_mutex_t mutex_logger;
pthread_mutex_t mutex_programs;
pthread_mutex_t mutex_threads;
pthread_mutex_t mutex_new;
pthread_mutex_t mutex_blocked;
pthread_mutex_t mutex_exit;

int main() {

    void* server_thread_error;
    void* metrics_thread_error;
    pthread_t server_thread;
    pthread_t metrics_thread;

    start_log();

    initialize_structures();

    read_config_options();

    pthread_create(&server_thread, NULL, server_function, NULL);
    pthread_create(&metrics_thread, NULL, metrics_function, NULL);

    pthread_join(server_thread, &server_thread_error);
    pthread_join(metrics_thread, &metrics_thread_error);

    //TODO:Loggear el error del servidor
    //int server_error = (int) (intptr_t) thread_server_error;

    return 0;
}

void start_log(){
    logger = log_create("../suse.log", "suse", 1, LOG_LEVEL_TRACE);
}

void read_config_options(){
    t_config* config_file = config_create("../suse.config");
    config->listen_port = config_get_int_value(config_file, "LISTEN_PORT");
    config->metrics_timer = config_get_int_value(config_file, "METRICS_TIMER");
    config->max_multiprog = config_get_int_value(config_file, "MAX_MULTIPROG");
    //config->sem_ids = config_get_array_value(config_file, "SEM_IDS");
    //config->sem_init = config_get_array_value(config_file, "SEM_INIT");
    //config->sem_max = config_get_array_value(config_file, "SEM_MAX");
    config->alpha_sjf = config_get_double_value(config_file, "ALPHA_SJF");
    log_trace(logger,
        "Configuración levantada: LISTEN_PORT: %d, METRICS_TIMER: %d, MAX_MULTIPROG: %d, ALPHA_SJF: %f.",
        config->listen_port,
        config->metrics_timer,
        config->max_multiprog,
        config->alpha_sjf);
    config_destroy(config_file);
}

void initialize_structures(){
    pthread_mutex_init(&mutex_logger, NULL);
    pthread_mutex_init(&mutex_programs, NULL);
    pthread_mutex_init(&mutex_threads, NULL);
    pthread_mutex_init(&mutex_new, NULL);
    pthread_mutex_init(&mutex_blocked, NULL);
    pthread_mutex_init(&mutex_exit, NULL);
    config = malloc(sizeof(SUSEConfig*));
    //TODO: averiguar como inicializar esto

    //config->sem_max = malloc(sizeof(char*));
    //config->sem_init = malloc(sizeof(char*));
    //config->sem_max = malloc(sizeof(char*));
    NEW = list_create();
    BLOCKED = list_create();
    EXIT = list_create();
    programs = list_create();
    pthread_mutex_lock(&mutex_logger);
    log_trace(logger, "Estructuras inicializadas...");
    pthread_mutex_unlock(&mutex_logger);
}

void* server_function(void * arg){
    int PORT = config->listen_port;
    int socket;
    if((socket = create_socket()) == -1) {
        printf("Error al crear el socket\n");
        return (void *) -1;
    }
    if((bind_socket(socket, PORT)) == -1) {
        printf("Error al bindear el socket\n");
        return (void *) -2;
    }

    //--Funcion que se ejecuta cuando se conecta un nuevo programa
    void new(int fd, char * ip, int port){
        create_new_program(ip, port);
    }

    //--Funcion que se ejecuta cuando se pierde la conexion con un cliente
    void lost(int fd, char * ip, int port){
        pthread_mutex_lock(&mutex_logger);
        log_trace(logger, "El programa en IP:%s, PORT:%d se ha desconectado", ip, port);
        pthread_mutex_unlock(&mutex_logger);
        //TODO:remove program from programs list
    }

    //--funcion que se ejecuta cuando se recibe un nuevo mensaje de un cliente ya conectado
    void incoming(int fd, char* ip, int port, MessageHeader * headerStruct){

        t_list* cosas = receive_package(fd, headerStruct);

        //TODO:voletear
        t_new_comm* newComm = malloc(sizeof(t_new_comm*));
        newComm->fd = fd;
        newComm->ip = malloc(sizeof(char*));
        newComm->ip = ip;
        newComm->port = port;
        newComm->received = malloc(sizeof(t_list*));
        newComm->received = cosas;

        switch (headerStruct->type){
            case SUSE_CREATE:
            {
                suse_create(fd, ip, port, cosas);
                break;
            }
            case SUSE_SCHEDULE_NEXT:
            {
                pthread_t suse_schedule_next_thread;
                pthread_create(&suse_schedule_next_thread, NULL, suse_schedule_next, (void*)newComm);
                pthread_detach(suse_schedule_next_thread);
                break;
            }
            case SUSE_WAIT:
            {
                pthread_t suse_wait_thread;
                pthread_create(&suse_wait_thread, NULL, suse_wait, (void*)newComm);
                pthread_detach(suse_wait_thread);
                break;
            }
            case SUSE_SIGNAL:
            {
                pthread_t suse_signal_thread;
                pthread_create(&suse_signal_thread, NULL, suse_signal, (void*)newComm);
                pthread_detach(suse_signal_thread);
                break;
            }
            case SUSE_JOIN:
            {
                pthread_t suse_join_thread;
                pthread_create(&suse_join_thread, NULL, suse_join, (void*)newComm);
                pthread_detach(suse_join_thread);
                break;
            }
            case SUSE_RETURN:
            {
                pthread_t suse_return_thread;
                pthread_create(&suse_return_thread, NULL, suse_return, (void*)newComm);
                pthread_detach(suse_return_thread);
                break;
            }
            default:
            {
                printf("Operacion desconocida. No quieras meter la pata\n");
                break;
            }
        }

    }
    pthread_mutex_lock(&mutex_logger);
    log_trace(logger, "Hilo de servidor iniciado...");
    pthread_mutex_unlock(&mutex_logger);
    start_server(socket, &new, &lost, &incoming);
}

//--LISTO
void create_new_program(char* ip, int port){
    PID pid = generate_pid(ip, port);
    t_programa* nuevo_programa = (t_programa*)malloc(sizeof(t_programa));
    nuevo_programa->pid = pid;
    nuevo_programa->ready = list_create();
    nuevo_programa->exec = malloc(sizeof(t_thread*));
    nuevo_programa->executing = false;

    pthread_mutex_lock(&mutex_programs);
    list_add(programs, (void*)nuevo_programa);
    pthread_mutex_unlock(&mutex_programs);

    pthread_mutex_lock(&mutex_logger);
    log_trace(logger, "Nuevo programa agregado, PID:%s", pid);
    pthread_mutex_unlock(&mutex_logger);
}

void* metrics_function(void* arg){
    pthread_mutex_lock(&mutex_logger);
    log_trace(logger, "Hilo de metricas iniciado...");
    pthread_mutex_unlock(&mutex_logger);
    while(1){
        sleep(config->metrics_timer);
        generate_metrics();
    }
}

void generate_metrics(){
    char* metric_to_log = string_new();
    string_append(&metric_to_log, "Metricas:\n");
    char* separator = "\n";
    char* thread_metrics = generate_thread_metrics();
    string_append(&metric_to_log, thread_metrics);
    string_append(&metric_to_log, separator);
    char* program_metrics = generate_program_metrics();
    string_append(&metric_to_log, program_metrics);
    string_append(&metric_to_log, separator);
    char* system_metrics = generate_system_metrics();
    string_append(&metric_to_log, system_metrics);
    string_append(&metric_to_log, separator);
    string_append(&metric_to_log, separator);

    pthread_mutex_lock(&mutex_logger);
    log_info(logger, metric_to_log);
    pthread_mutex_unlock(&mutex_logger);

    free(metric_to_log);
}

char* generate_thread_metrics(){
    char* metrics = "Thread metrics\n";
    return metrics;
}

char* generate_program_metrics(){
    char* metrics = "Program metrics\n";
    return metrics;
}

char* generate_system_metrics(){
    char* metrics = "System metrics";
    return metrics;
}

void suse_create(int fd, char * ip, int port, t_list* received){
    char* pid = generate_pid(ip, port);
    int tid = *((int*)list_get(received, 0));

    t_thread* new_thread = malloc(sizeof(t_thread*));
    new_thread->tid = tid;
    new_thread->pid = pid;
    new_thread->exec_list = list_create();
    new_thread->ready_list = list_create();
    new_thread->start_time = get_time();

    if(multiprogramming_grade() >= config->max_multiprog){
        list_add(NEW, (void*)new_thread);
        pthread_mutex_lock(&mutex_logger);
        log_trace(logger, "Hilo, TID:%d, del programa, PID:%s, agregado a la lista de new", tid, pid);
        pthread_mutex_unlock(&mutex_logger);
    } else {
        bool program_finder(void* program){
            return strcmp(((t_programa*)program)->pid, pid) == 0;
        }

        t_programa* program = (t_programa*)list_find(programs, &program_finder);
        if(program->executing) {
            t_list *ready = program->ready;
            list_add(ready, new_thread);

            pthread_mutex_lock(&mutex_logger);
            log_trace(logger, "Hilo, TID:%d, del programa, PID:%s, agregado a la lista de ready", tid, pid);
            pthread_mutex_unlock(&mutex_logger);
        } else {
            interval* first_execution = malloc(sizeof(interval*));
            first_execution->start_time = get_time();
            t_list* exec_time = new_thread->exec_list;
            list_add(exec_time, (void*)first_execution);
            program->exec = new_thread;
            program->executing = true;

            pthread_mutex_lock(&mutex_logger);
            log_trace(logger, "Hilo, TID:%d, del programa, PID:%s, ahora ejecutandose", tid, pid);
            pthread_mutex_unlock(&mutex_logger);
        }
    }

    //Confirmo la planificacion del hilo
    t_paquete *package = create_package(SUSE_CREATE);
    void* confirmation = malloc(sizeof(int));
    *((int*)confirmation) = 1;
    add_to_package(package, confirmation, sizeof(int) + 1);
    send_package(package, fd);

}

void* suse_schedule_next(void* newComm){
    t_new_comm* newComm1 = (t_new_comm*)newComm;
    int fd = newComm1->fd;
    char* ip = newComm1->ip;
    int port = newComm1->port;
    t_list* received = newComm1->received;
}

void* suse_wait(void* newComm){
    t_new_comm* newComm1 = (t_new_comm*)newComm;
    int fd = newComm1->fd;
    char* ip = newComm1->ip;
    int port = newComm1->port;
    t_list* received = newComm1->received;
}

void* suse_signal(void* newComm){
    t_new_comm* newComm1 = (t_new_comm*)newComm;
    int fd = newComm1->fd;
    char* ip = newComm1->ip;
    int port = newComm1->port;
    t_list* received = newComm1->received;
}

void* suse_join(void* newComm){
    t_new_comm* newComm1 = (t_new_comm*)newComm;
    int fd = newComm1->fd;
    char* ip = newComm1->ip;
    int port = newComm1->port;
    t_list* received = newComm1->received;
}

void* suse_return(void* newComm){
    t_new_comm* newComm1 = (t_new_comm*)newComm;
    int fd = newComm1->fd;
    char* ip = newComm1->ip;
    int port = newComm1->port;
    t_list* received = newComm1->received;
}

//--Helpers

char* generate_pid(char* ip, int port){
    char* new_pid = string_new();
    char* separator = "::";
    char* string_port = string_itoa(port);
    string_append(&new_pid, ip);
    string_append(&new_pid, separator);
    string_append(&new_pid, string_port);
    return new_pid;
}

int multiprogramming_grade(){
    int blocked_grade = list_size(BLOCKED);

    //--Readies

    //--Mapeo un programa al tamaño de su lista de ready
    void* program_to_ready_grade(void* program){
        t_list* ready = ((t_programa*)program)->ready;
        void* grade = malloc(sizeof(int));
        *((int*)grade) = list_size(ready);
        return grade;
    }

    t_list* ready_grades = list_map(programs, &program_to_ready_grade);
    void* seed = malloc(sizeof(int));
    *((int*)seed) = 0;


    void* seed_plus_grade(void* seed, void* grade){
        void* newSeed = malloc(sizeof(int));
        *((int*)newSeed) = *((int*) seed) + *((int*) grade);
        return newSeed;
    }

    //--Sumo la lista mapeada de arriba con una semilla inicial de 0
    int ready_grade = *((int*)list_fold(ready_grades, seed, &seed_plus_grade));

    //--Execs
    bool executing_program(void* program){
        return ((t_programa*)program)->executing;
    }

    int execute_grade = list_count_satisfying(programs, &executing_program);

    return blocked_grade + ready_grade + execute_grade;
}

struct timespec get_time(){
    struct timespec start;
    clock_gettime(CLOCK_MONOTONIC_RAW, &start);
    return start;
}