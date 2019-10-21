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
        create_new_program(ip, port, fd);
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
            case SUSE_CLOSE:
            {
                suse_close(fd, ip, port, cosas);
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

//--LISTO
void create_new_program(char* ip, int port, int fd){
    PID pid = generate_pid(ip, port);
    t_programa* nuevo_programa = (t_programa*)malloc(sizeof(t_programa));
    nuevo_programa->pid = pid;
    nuevo_programa->fd = fd;
    nuevo_programa->ready = list_create();
    nuevo_programa->exec = malloc(sizeof(t_thread));
    nuevo_programa->executing = false;

    pthread_mutex_lock(&mutex_programs);
    list_add(programs, (void*)nuevo_programa);
    pthread_mutex_unlock(&mutex_programs);

    pthread_mutex_lock(&mutex_logger);
    log_trace(logger, "New program added, PID:%s", pid);
    pthread_mutex_unlock(&mutex_logger);
}

//--LISTO
void suse_create(int fd, char * ip, int port, t_list* received){
    char* pid = generate_pid(ip, port);
    int tid = *((int*)list_get(received, 0));

    t_thread* new_thread = malloc(sizeof(t_thread));
    new_thread->tid = tid;
    new_thread->pid = pid;
    new_thread->exec_list = list_create();
    new_thread->ready_list = list_create();
    new_thread->start_time = malloc(sizeof(struct timespec));
    *(new_thread->start_time) = get_time();

    if(multiprogramming_grade() >= config->max_multiprog){
        list_add(NEW, (void*)new_thread);
        pthread_mutex_lock(&mutex_logger);
        log_trace(logger, "Program(%s)'s Thread(%d) added to NEW list", pid, tid);
        pthread_mutex_unlock(&mutex_logger);
    } else {
        t_programa* program = find_program(pid);

        interval* first_iteration = new_interval();
        *(first_iteration->start_time) = get_time();

        if(program->executing) { //And if exec!= NULL?
            t_list *ready = program->ready;
            list_add(ready, new_thread);

            list_add(new_thread->ready_list, (void*)first_iteration);

            pthread_mutex_lock(&mutex_logger);
            log_trace(logger, "Thread(%d), added to Program(%s)'s ready list", tid, pid);
            pthread_mutex_unlock(&mutex_logger);
        } else {
            list_add(new_thread->exec_list, (void*)first_iteration);
            program->exec = new_thread;
            program->executing = true;

            pthread_mutex_lock(&mutex_logger);
            log_trace(logger, "Program(%s)'s Thread(%d) now executing", pid, tid);
            pthread_mutex_unlock(&mutex_logger);
        }
    }

    //Confirmo la planificacion del hilo
    create_response_thread(fd, 1, SUSE_CREATE);

    void element_destroyer(void* element){
        free(element);
    }
    free_list(received, element_destroyer);
}

void suse_schedule_next(int fd, char * ip, int port, t_list* received){
    char* pid = generate_pid(ip, port);
    t_programa* program = find_program(pid);
    int return_tid;

    //Verifico que el programa exista(que no se haya cerrado con suse_close)
    if(program != NULL) {

        pthread_mutex_lock(&mutex_logger);
        log_trace(logger, "Program(%s), asking for new thread", pid);
        pthread_mutex_unlock(&mutex_logger);

        return_tid = schedule_next(program);
    } else {
        return_tid = -1;
    }

    //Confirmo la planificacion del hilo
    create_response_thread(fd, return_tid, SUSE_SCHEDULE_NEXT);

    void element_destroyer(void* element){
        free(element);
    }
    free_list(received, element_destroyer);
    free((void*)pid);
}

int schedule_next(t_programa* program){
    int return_tid;

    //Si no esta en ejecucion significa que todos sus hilos estan en el estado new, aun no puedo hacer nada
    if(program->executing){
        t_list* ready = program->ready;
        if(list_size(ready) > 0){
            //Tomo el primer hilo de la lista de readys(FIFO)TODO:implementar SJF-E
            t_thread* thread_to_execute = (t_thread*)list_remove(ready, 0);

            return_tid = thread_to_execute->tid;

            struct timespec* change_time = malloc(sizeof(struct timespec));
            *(change_time) = get_time();

            interval* new_start = new_interval();
            new_start->start_time = change_time;

            list_add(thread_to_execute->exec_list, (void*)new_start);

            //Si el hilo ejecutado anterior fue cerrado(suse_close), el campo estara en NULL
            if(program->exec != NULL){

                //Obtengo el ultimo elemento(interval) de la lista de ejecutados del hilo actualmente en ejecucion
                interval* last_execution_OLDONE = last_exec(program->exec);

                *(last_execution_OLDONE->end_time) = *(change_time);

                //Creo un nuevo elemento para la lista ready, le asigno el tiempo de inicio y lo agrego a la lista de readys del elemento en ejecucion
                interval* new_ready_OLDONE = new_interval();

                *(new_ready_OLDONE->start_time) = *(change_time);

                list_add(program->exec->ready_list, new_ready_OLDONE);

                //Obtengo el ultimo elemento(interval) de la lista de readys del proximo hilo a ejecutar y le agrego el tiempo final
                interval* last_ready_NEWONE = last_ready(thread_to_execute);

                *(last_ready_NEWONE->end_time) = *(change_time);

                //Agrego el hilo en ejecucion a la lista de readys y le asigno a exec el nuevo hilo a ejecutar
                TID old_tid = program->exec->tid;
                list_add(program->ready, (void*)program->exec);
                program->exec = thread_to_execute;
                TID new_tid = program->exec->tid;

                pthread_mutex_lock(&mutex_logger);
                log_trace(logger, "Thread: %d, exchanged for Thread: %d on Program: %s", old_tid, new_tid, program->pid);
                pthread_mutex_unlock(&mutex_logger);
            } else {

                //Obtengo el ultimo elemento(interval) de la lista de readys del proximo hilo a ejecutar y le agrego el tiempo final
                interval* last_ready_NEWONE = last_ready(thread_to_execute);

                *(last_ready_NEWONE->end_time) = *(change_time);

                //Le asigno a exec el nuevo hilo a ejecutar
                program->exec = thread_to_execute;
                TID new_tid = program->exec->tid;

                pthread_mutex_lock(&mutex_logger);
                log_trace(logger, "Thread: %d, now executing on Program: %s", new_tid, program->pid);
                pthread_mutex_unlock(&mutex_logger);
            }

        } else {
            //TODO: si no hay mas hilos, que retorno?
            return_tid = program->exec->tid;
        }
    } else {
        //TODO: si el programa no esta ejecutando(porque todos sus hilos estan en new, que retorno?, -1?)
        return_tid = -1;
    }

    return return_tid;
}

void suse_close(int fd, char * ip, int port, t_list* received){
    char* pid = generate_pid(ip, port);
    int tid = *((int*)list_get(received, 0));
    t_programa* program = find_program(pid);

    pthread_mutex_lock(&mutex_logger);
    log_trace(logger, "Program(%s), asking to finish thread: %d", pid, tid);
    pthread_mutex_unlock(&mutex_logger);



    //TODO:si el programa muere, mandar un 1, si se planifica un nuevo hilo, un 2
    create_response_thread(fd, 1, SUSE_CLOSE);

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

void create_response_thread(int fd, int response, MessageType header){
    void* response_package = create_response_package(fd, response, header);

    pthread_t response_thread;
    pthread_create(&response_thread, NULL, response_function, response_package);
    pthread_detach(response_thread);
}

void* create_response_package(int fd, int response, MessageType header){
    t_new_response* response_package = malloc(sizeof(t_new_response));
    response_package->fd = fd;
    response_package->response = response;
    response_package->header = header;

    return (void*)response_package;
}

void* response_function(void* response_package){
    t_new_response* new_response_package = (t_new_response*)response_package;
    int fd = new_response_package->fd;
    int response = new_response_package->response;
    MessageType header = new_response_package->header;

    t_paquete *package = create_package(header);
    void* confirmation = malloc(sizeof(int));
    *((int*)confirmation) = response;
    add_to_package(package, confirmation, sizeof(int));
    send_package(package, fd);
    free(confirmation);
    free_package(package);
    free(new_response_package);
}

interval* last_exec(t_thread* thread){
    t_list* exec_list = thread->exec_list;
    int exec_list_size = list_size(exec_list) - 1;
    return (interval*)list_get(exec_list, exec_list_size);
}

interval* last_ready(t_thread* thread){
    t_list* ready_list = thread->ready_list;
    int ready_list_size = list_size(ready_list) - 1;
    void* last_ready = list_get(ready_list, ready_list_size);
    return (interval*)last_ready;
}

interval* new_interval(){
    interval* iteration = malloc(sizeof(interval));
    iteration->start_time = malloc(sizeof(struct timespec));
    iteration->end_time = malloc(sizeof(struct timespec));
    return iteration;
}