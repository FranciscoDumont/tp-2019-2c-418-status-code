#include "suse.h"

SUSEConfig* config;
t_log* logger;
t_list* programs;
t_list* NEW;
t_list* BLOCKED;

pthread_mutex_t mutex_logger;
pthread_mutex_t mutex_programs;

int main() {
    pthread_t metrics_thread;
    void* metrics_thread_error;

    start_log();

    initialize_structures();

    read_config_options();

    pthread_create(&metrics_thread, NULL, metrics_function, NULL);

    server_function();

    pthread_join(metrics_thread, &metrics_thread_error);

    return 0;
}

void start_log(){
    logger = log_create("../suse.log", "suse", 1, LOG_LEVEL_TRACE);
}

void initialize_structures(){
    pthread_mutex_init(&mutex_logger, NULL);
    pthread_mutex_init(&mutex_programs, NULL);

    config = malloc(sizeof(SUSEConfig));
    programs = list_create();
    NEW = list_create();
    BLOCKED = list_create();

    pthread_mutex_lock(&mutex_logger);
    log_trace(logger, "Structures initialized...");
    pthread_mutex_unlock(&mutex_logger);
}

void read_config_options(){
    t_config* config_file = config_create("../suse.config");
    config->listen_port = config_get_int_value(config_file, "LISTEN_PORT");
    config->metrics_timer = config_get_int_value(config_file, "METRICS_TIMER");
    config->max_multiprog = config_get_int_value(config_file, "MAX_MULTIPROG");
    config->alpha_sjf = config_get_double_value(config_file, "ALPHA_SJF");
    log_trace(logger,
              "Config file read: LISTEN_PORT: %d, METRICS_TIMER: %d, MAX_MULTIPROG: %d, ALPHA_SJF: %f.",
              config->listen_port,
              config->metrics_timer,
              config->max_multiprog,
              config->alpha_sjf);
    config_destroy(config_file);
}

void server_function(){
    int PORT = config->listen_port;
    int socket;
    if((socket = create_socket()) == -1) {
        printf("Error creating socket\n");
        return;
    }
    if((bind_socket(socket, PORT)) == -1) {
        printf("Error binding socket\n");
        return;
    }

    //--Funcion que se ejecuta cuando se conecta un nuevo programa
    void new(int fd, char * ip, int port){
        create_new_program(ip, port);
    }

    //--Funcion que se ejecuta cuando se pierde la conexion con un cliente
    void lost(int fd, char * ip, int port){
        pthread_mutex_lock(&mutex_logger);
        log_trace(logger, "Program on IP:%s, PORT:%d has disconnected", ip, port);
        pthread_mutex_unlock(&mutex_logger);
        //TODO:remove program from programs list
        //TODO:free all resources from the program
    }

    //--funcion que se ejecuta cuando se recibe un nuevo mensaje de un cliente ya conectado
    void incoming(int fd, char* ip, int port, MessageHeader * headerStruct){

        //TODO: free this list within every message
        t_list* cosas = receive_package(fd, headerStruct);

        switch (headerStruct->type){
            case SUSE_CREATE:
            {
                suse_create(fd, ip, port, cosas);
                break;
            }
            case SUSE_SCHEDULE_NEXT:
            {
                suse_schedule_next(fd, ip, port, cosas);
                break;
            }
            default:
            {
                printf("Unknown operation\n");
                break;
            }
        }

    }
    pthread_mutex_lock(&mutex_logger);
    log_trace(logger, "Server initiated...");
    pthread_mutex_unlock(&mutex_logger);
    start_server(socket, &new, &lost, &incoming);
}

void create_new_program(char* ip, int port){
    PID pid = generate_pid(ip, port);
    t_programa* nuevo_programa = (t_programa*)malloc(sizeof(t_programa));
    nuevo_programa->pid = pid;
    nuevo_programa->ready = list_create();
    nuevo_programa->exec = malloc(sizeof(t_thread));
    nuevo_programa->executing = false;

    pthread_mutex_lock(&mutex_programs);
    list_add(programs, (void*)nuevo_programa);
    pthread_mutex_unlock(&mutex_programs);

    pthread_mutex_lock(&mutex_logger);
    log_trace(logger, "Nuevo programa agregado, PID:%s", pid);
    pthread_mutex_unlock(&mutex_logger);
}

void suse_create(int fd, char * ip, int port, t_list* received){
    char* pid = generate_pid(ip, port);
    int tid = *((int*)list_get(received, 0));

    t_thread* new_thread = malloc(sizeof(t_thread));
    new_thread->tid = tid;
    new_thread->pid = pid;
    new_thread->exec_list = list_create();
    new_thread->ready_list = list_create();
    new_thread->start_time = get_time();

    if(multiprogramming_grade() >= config->max_multiprog){
        list_add(NEW, (void*)new_thread);
        pthread_mutex_lock(&mutex_logger);
        log_trace(logger, "Program(%s)'s Thread(%d) added to NEW list", pid, tid);
        pthread_mutex_unlock(&mutex_logger);
    } else {
        t_programa* program = find_program(pid);
        if(program->executing) {
            t_list *ready = program->ready;
            list_add(ready, new_thread);

            pthread_mutex_lock(&mutex_logger);
            log_trace(logger, "Thread(%d), added to Program(%s)'s ready list", tid, pid);
            pthread_mutex_unlock(&mutex_logger);
        } else {
            interval* first_execution = malloc(sizeof(interval));
            first_execution->start_time = get_time();
            t_list* exec_time = new_thread->exec_list;
            list_add(exec_time, (void*)first_execution);
            program->exec = new_thread;
            program->executing = true;

            pthread_mutex_lock(&mutex_logger);
            log_trace(logger, "Program(%s)'s Thread(%d) now executing", pid, tid);
            pthread_mutex_unlock(&mutex_logger);
        }
    }

    //Confirmo la planificacion del hilo
    t_paquete *package = create_package(SUSE_CREATE);
    void* confirmation = malloc(sizeof(int));
    *((int*)confirmation) = 1;
    add_to_package(package, confirmation, sizeof(int));
    send_package(package, fd);
    free(confirmation);
    free_package(package);

    void element_destroyer(void* element){
        free(element);
    }
    free_list(received, element_destroyer);
}

void suse_schedule_next(int fd, char * ip, int port, t_list* received){
    char* pid = generate_pid(ip, port);
    t_programa* program = find_program(pid);

    pthread_mutex_lock(&mutex_logger);
    log_trace(logger, "Program(%s), asking for new thread", pid);
    pthread_mutex_unlock(&mutex_logger);

    int return_tid;

    if(program->executing){
        //TODO: implementar SJFE
        t_list* ready = program->ready;
        if(list_size(ready) > 0){
            //Take thread from ready list and instantiate a new interval for the execution
            t_thread* thread_to_execute = (t_thread*)list_remove(ready, 0);

            return_tid = thread_to_execute->tid;

            interval* new_execution = malloc(sizeof(interval));
            new_execution->start_time = get_time();

            list_add(thread_to_execute->exec_list, (void*)new_execution);

            exchange_executing_threads(program, thread_to_execute);

        } else {
            //TODO: si no hay mas hilos, que retorno?
            return_tid = program->exec->tid;
        }
    } else {
        //TODO: si el programa no esta ejecutando(porque todos sus hilos estan en new, que retorno?, -1?)
        return_tid = -1;
    }

    //Confirmo la planificacion del hilo
    t_paquete *package = create_package(SUSE_SCHEDULE_NEXT);
    void* confirmation = malloc(sizeof(int));
    *((int*)confirmation) = return_tid;
    add_to_package(package, confirmation, sizeof(int));
    send_package(package, fd);
    free(confirmation);
    free_package(package);

    void element_destroyer(void* element){
        free(element);
    }
    free_list(received, element_destroyer);
}

void* metrics_function(void* arg){
    pthread_mutex_lock(&mutex_logger);
    log_trace(logger, "Metrics thread initiated...");
    pthread_mutex_unlock(&mutex_logger);
    while(1){
        sleep(config->metrics_timer);
        generate_metrics();
    }
}

void generate_metrics(){
    char* metric_to_log = string_new();
    string_append(&metric_to_log, "---------METRICS:\n");
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

    pthread_mutex_lock(&mutex_logger);
    log_info(logger, metric_to_log);
    pthread_mutex_unlock(&mutex_logger);

    free(metric_to_log);
}

char* generate_thread_metrics(){
    char* metrics = "Thread metrics:\n";
    return metrics;
}

char* generate_program_metrics(){
    char* metrics = "Program metrics:\n";
    return metrics;
}

char* generate_system_metrics(){
    char* metrics = "System metrics:\n";
    return metrics;
}

//--Helpers

char* generate_pid(char* ip, int port){
    char* new_pid = string_new();
    char* separator = "::";
    char* string_port = string_itoa(port);
    string_append(&new_pid, ip);
    string_append(&new_pid, separator);
    string_append(&new_pid, string_port);
    free(string_port);
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
        *((int*) seed) += *((int*) grade);
        free(grade);
        return seed;
    }

    //--Sumo la lista mapeada de arriba con una semilla inicial de 0
    void* ready_grade_ptr = list_fold(ready_grades, seed, &seed_plus_grade);
    int ready_grade = *((int*)ready_grade_ptr);
    free(ready_grade_ptr);
    list_destroy(ready_grades);

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

void free_list(t_list* received, void(*element_destroyer)(void*)){
    list_destroy_and_destroy_elements(received, element_destroyer);
}

t_programa* find_program(PID pid){
    bool program_finder(void* program){
        return strcmp(((t_programa*)program)->pid, pid) == 0;
    }
    return (t_programa*)list_find(programs, &program_finder);
}

void exchange_executing_threads(t_programa* program, t_thread* new_one){
    t_list* exec_list_OLDONE = program->exec->exec_list;
    int exec_list_size_OLDONE = list_size(exec_list_OLDONE) - 1;
    interval* last_execution_OLDONE = list_get(exec_list_OLDONE, exec_list_size_OLDONE);

    t_list* exec_list_NEWONE = new_one->exec_list;
    int exec_list_size_NEWONE = list_size(exec_list_NEWONE) - 1;
    interval* last_execution_NEWONE = list_get(exec_list_NEWONE, exec_list_size_NEWONE);

    memcpy((void*)&last_execution_OLDONE->end_time, (void*)&last_execution_NEWONE->start_time, sizeof(interval));

    TID old_tid = program->exec->tid;
    list_add(program->ready, (void*)program->exec);
    program->exec = new_one;
    TID new_tid = program->exec->tid;

    pthread_mutex_lock(&mutex_logger);
    log_trace(logger, "Thread: %d, exchanged for Thread: %d", old_tid, new_tid);
    pthread_mutex_unlock(&mutex_logger);
}