#include "suse.h"

SUSEConfig* config;
t_log* logger;
t_list* programs;
t_list* NEW;
t_list* BLOCKED;
t_list* EXIT; //Solo se elimina un hilo y su programa cuando todos los hilos del mismo son enviados a EXIT

pthread_mutex_t mutex_logger;
pthread_mutex_t mutex_programs;

//--LISTO
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

//--LISTO
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
    EXIT = list_create();

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
    }

    //--funcion que se ejecuta cuando se recibe un nuevo mensaje de un cliente ya conectado
    void incoming(int fd, char* ip, int port, MessageHeader * headerStruct){

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
            case SUSE_JOIN:
            {
                suse_join(fd, ip, port, cosas);
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
    t_program* new_program = (t_program*)malloc(sizeof(t_program));
    new_program->pid = pid;
    new_program->fd = fd;
    new_program->ready = list_create();
    new_program->exec = malloc(sizeof(t_thread));
    new_program->executing = false;

    pthread_mutex_lock(&mutex_programs);
    list_add(programs, (void*)new_program);
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
        t_program* program = find_program(pid);

        t_interval* first_iteration = new_interval();
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
    t_program* program = find_program(pid);
    int return_tid;

    //Verifico que el programa exista(que no se haya cerrado con suse_close)
    if(program != NULL) {

        pthread_mutex_lock(&mutex_logger);
        log_trace(logger, "Program: %s, asking to schedule a new thread", pid);
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

int schedule_next(t_program* program){
    int return_tid;

    //Si no esta en ejecucion significa que todos sus hilos estan en el estado new, aun no puedo hacer nada
    if(program->executing){
        t_list* ready = program->ready;
        if(list_size(ready) > 0){

            t_thread* thread_to_execute = schedule_new_thread(program);

            return_tid = thread_to_execute->tid;

            t_interval* new_start = new_interval();
            *(new_start->start_time) = get_time();

            list_add(thread_to_execute->exec_list, (void*)new_start);

            //Si el hilo ejecutado anterior fue cerrado(suse_close) o bloqueado(suse_wait, suse_join), el campo estara en NULL
            if(program->exec != NULL){

                //Obtengo el ultimo elemento(interval) de la lista de ejecutados del hilo actualmente en ejecucion
                t_interval* last_execution_OLDONE = last_exec(program->exec);

                *(last_execution_OLDONE->end_time) = *(new_start->start_time);

                //Creo un nuevo elemento para la lista ready, le asigno el tiempo de inicio y lo agrego a la lista de readys del elemento en ejecucion
                t_interval* new_ready_OLDONE = new_interval();

                *(new_ready_OLDONE->start_time) = *(new_start->start_time);

                list_add(program->exec->ready_list, new_ready_OLDONE);

                //Obtengo el ultimo elemento(interval) de la lista de readys del proximo hilo a ejecutar y le agrego el tiempo final
                t_interval* last_ready_NEWONE = last_ready(thread_to_execute);

                *(last_ready_NEWONE->end_time) = *(new_start->start_time);

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
                t_interval* last_ready_NEWONE = last_ready(thread_to_execute);

                *(last_ready_NEWONE->end_time) = *(new_start->start_time);

                //Le asigno a exec el nuevo hilo a ejecutar
                program->exec = thread_to_execute;
                TID new_tid = program->exec->tid;
            }

        } else {
            //Si no hay mas hilos en ready, retorno el mismo tid
            return_tid = program->exec->tid;
        }
    } else {
        //TODO: si el programa no esta ejecutando(porque todos sus hilos estan en new, que retorno?, -1?)
        return_tid = -1;
    }

    pthread_mutex_lock(&mutex_logger);
    log_trace(logger, "Thread: %d, now executing on Program: %s", return_tid, program->pid);
    pthread_mutex_unlock(&mutex_logger);

    return return_tid;
}

//TODO:implementar SJF-E, actualmente es FIFO
t_thread* schedule_new_thread(t_program* program){
    return (t_thread*)list_remove(program->ready, 0);
}

void suse_join(int fd, char * ip, int port, t_list* received){
    char* pid = generate_pid(ip, port);
    int tid = *((int*)list_get(received, 0));
    t_program* program = find_program(pid);
    int response;

    //Obtengo el hilo en ejecucion
    t_thread* executing_thread = program->exec;
    int executing_tid = executing_thread->tid;

    pthread_mutex_lock(&mutex_logger);
    log_trace(logger, "Program: %s, asking to join Thread: %d with Thread: %d", pid, executing_tid, tid);
    pthread_mutex_unlock(&mutex_logger);

    //Obtengo el ultimo intervalo de ejecucion y le asigno su tiempo de finalizacion
    t_interval* last_execd = last_exec(executing_thread);
    *(last_execd->end_time) = get_time();

    //Busco el hilo que me quiere bloquear
    t_thread* blocking_thread = find_thread(program, tid);

    //Si el hilo bloqueante esta en EXIT
    if(blocking_thread_is_dead(blocking_thread)){

        //Creo un nuevo intervalo y le asigno el tiempo de inicio para agregar a la lista de ready del hilo
        t_interval* new_ready = new_interval();
        *(new_ready->start_time) = *(last_execd->end_time);

        list_add(executing_thread->ready_list, (void*)new_ready);

        //Agrego el hilo que estaba ejecutandose a la lista de listos
        list_add(program->ready, (void*)executing_thread);

        pthread_mutex_lock(&mutex_logger);
        log_trace(logger, "Blocking Thread(%d) was dead, blocked Thread(%d) sent to ready", tid, executing_tid);
        pthread_mutex_unlock(&mutex_logger);
    } else {

        //Creo un nuevo blockeo y le asigno el tipo
        t_block* new_block = malloc(sizeof(t_block));
        new_block->block_type = JOIN;

        //Creo la estructura encargada de representar los bloqueos por join y le asigno el hilo bloqueado y bloqueante
        t_join_block* new_join_block = malloc(sizeof(t_join_block));

        new_join_block->blocked_thread = executing_thread;
        new_join_block->blocking_thread = blocking_thread;

        //Le asigno el blockeo de join a la estructura del bloqueo
        new_block->block_structure = (void*)new_join_block;

        //Agrego el nuevo blockeo a la lista de BLOCKED
        list_add(BLOCKED, (void*)new_block);

        pthread_mutex_lock(&mutex_logger);
        log_trace(logger, "Thread: %d blocked by a join with Thread: %d", executing_tid, tid);
        pthread_mutex_unlock(&mutex_logger);
    }

    program->exec = NULL;

    create_response_thread(fd, 1, SUSE_JOIN);

    void element_destroyer(void* element){
        free(element);
    }
    free_list(received, element_destroyer);
    free((void*)pid);
}

void suse_close(int fd, char * ip, int port, t_list* received){
    char* pid = generate_pid(ip, port);
    int tid = *((int*)list_get(received, 0));
    t_program* program = find_program(pid);
    int response;

    pthread_mutex_lock(&mutex_logger);
    log_trace(logger, "Program: %s, asking to finish thread: %d", pid, tid);
    pthread_mutex_unlock(&mutex_logger);

    t_thread* exec_thread = program->exec;

    //Es posible que el hilo a cerrar no sea el que este en ejecucion?
    if(exec_thread->tid == tid) {

        program->exec = NULL;

        //Cierro el ultimo intervalo de ejecucion del hilo y lo agrego a la lista EXIT
        t_interval* last_execd = last_exec(exec_thread);
        *(last_execd->end_time) = get_time();

        list_add(EXIT, (void*)exec_thread);

        //Libero los bloqueos que haya generado el hilo
        free_join_blocks(exec_thread, program);

        //Mato al thread
        //destroy_thread(exec_thread);

        pthread_mutex_lock(&mutex_logger);
        log_trace(logger, "Thread: %d, from Program: %s, sent to EXIT", tid, pid);
        pthread_mutex_unlock(&mutex_logger);

        //Verifico si no quedan mas hilos en ready o exec
        if(no_more_threads(program)){

            destroy_exit_threads(pid);

            //Destruyo el programa
            destroy_program(pid);
            pthread_mutex_lock(&mutex_logger);
            log_trace(logger, "Program: %s, exited SUSE", pid);
            pthread_mutex_unlock(&mutex_logger);
        }

        //Lanzo un hilo detacheable que genera metricas
        pthread_t end_thread_metrics_thread;
        pthread_create(&end_thread_metrics_thread, NULL, generate_metrics, NULL);
        pthread_detach(end_thread_metrics_thread);

        //TODO:agregar el/los hilo/s que esten en new hasta cumplir con el grado de multiprogramacion a sus respectivos programas

        response = 1;
    } else {
        //El hilo a cerrar no era el que estaba en ejecucion
        response = -1;
    }

    free(pid);

    //1 para exito, -1 en el caso de error
    create_response_thread(fd, response, SUSE_CLOSE);

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
        generate_metrics(NULL);
    }
}

void* generate_metrics(void* arg){
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
        t_list* ready = ((t_program*)program)->ready;
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
        return ((t_program*)program)->executing;
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

t_program* find_program(PID pid){
    bool program_finder(void* program){
        return strcmp(((t_program*)program)->pid, pid) == 0;
    }
    return (t_program*)list_find(programs, &program_finder);
}

t_thread* find_thread(t_program* program, TID tid){
    t_thread* thread = NULL;
    PID pid = program->pid;

    //Busco el hilo en la lista de ready del programa al que pertenece
    bool ready_thread_finder(void* _thread){
        return ((t_thread*)_thread)->tid == tid;
    }
    thread = (t_thread*)list_find(program->ready, ready_thread_finder);

    //Si el hilo no se encuentra en la lista de readys del programa, busco en EXIT
    if(thread == NULL){
        //Busco el hilo en EXIT
        bool exit_thread_finder(void* _thread){
            return ((t_thread*)_thread)->tid == tid && strcmp(((t_thread*)_thread)->pid, program->pid) == 0;
        }
        thread = (t_thread*)list_find(program->ready, exit_thread_finder);

        //TODO:verificar que no sea NULL y si lo es, buscar en la lista de BLOCKED
    }

    return thread;
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

t_interval* last_exec(t_thread* thread){
    t_list* exec_list = thread->exec_list;
    int exec_list_size = list_size(exec_list) - 1;
    return (t_interval*)list_get(exec_list, exec_list_size);
}

t_interval* last_ready(t_thread* thread){
    t_list* ready_list = thread->ready_list;
    int ready_list_size = list_size(ready_list) - 1;
    return (t_interval*)list_get(ready_list, ready_list_size);
}

t_interval* new_interval(){
    t_interval* iteration = malloc(sizeof(t_interval));
    iteration->start_time = malloc(sizeof(struct timespec));
    iteration->end_time = malloc(sizeof(struct timespec));
    return iteration;
}

void destroy_thread(t_thread* thread){
    void interval_destroyer(void* _interval){
        t_interval* interval = (t_interval*)_interval;
        free(interval->end_time);
        free(interval->start_time);
        free(interval);
    }
    free_list(thread->ready_list, interval_destroyer);
    free_list(thread->exec_list, interval_destroyer);
    free(thread->start_time);
    free(thread->pid);
    free(thread);
}

bool no_more_threads(t_program* program){
    return (threads_in_new(program) + threads_in_ready(program) + threads_in_blocked(program) + threads_in_exec(program)) == 0;
}

int threads_in_new(t_program* program){
    bool condition(void* _thread){
        t_thread* thread = (t_thread*)_thread;
        return strcmp(program->pid, thread->pid) == 0;
    }
    return list_count_satisfying(NEW, condition);
}

int threads_in_ready(t_program* program){
    return list_size(program->ready);
}

int threads_in_blocked(t_program* program){
    return threads_in_join_block(program) + threads_in_semaphore_block(program);
}

int threads_in_join_block(t_program* program){
    bool condition(void* _block){
        t_block* block = (t_block*)_block;
        t_join_block* join_block = (t_join_block*)(block->block_structure);
        t_thread* blocked_thread = join_block->blocked_thread;
        return block->block_type == JOIN && strcmp(blocked_thread->pid, program->pid) == 0;
    }
    return list_count_satisfying(BLOCKED, condition);
}

//TODO:Implementar
int threads_in_semaphore_block(t_program* program){
    return 0;
}

int threads_in_exec(t_program* program){
    if(program->exec != NULL){
        return 1;
    }
    return 0;
}

void destroy_program(PID pid){

    bool condition(void* _program){
        t_program* program = (t_program*)_program;
        return strcmp(pid, program->pid) == 0;
    }
    void element_destroyer(void* _program){
        t_program* program = (t_program*)_program;
        free(program->pid);
        free(program);
    }
    list_remove_and_destroy_by_condition(programs, condition, element_destroyer);
}

void destroy_exit_threads(PID pid){
    bool execute = true;
    while(execute){
        bool condition(void* _thread){
            t_thread* thread1 = (t_thread*)_thread;
            return strcmp(thread1->pid, pid) == 0;
        }
        t_thread* thread = (t_thread*)list_remove_by_condition(EXIT, condition);
        if(thread == NULL){
            execute = false;
        } else {
            destroy_thread(thread);
        }
    }
}

bool blocking_thread_is_dead(t_thread* thread){
    bool condition(void* _thread){
        t_thread* thread_to_compare = (t_thread*)_thread;
        return thread_to_compare->tid == thread->tid && strcmp(thread_to_compare->pid, thread->pid) == 0;
    }
    return list_any_satisfy(EXIT, condition);
}

void free_join_blocks(t_thread* thread, t_program* program){
    bool execute = true;

    //Mientras que siga habiendo elementos, sigo iterando sobre la lista de BLOCKED
    while(execute){
        bool condition(void* _block){
            t_block* block = (t_block*)_block;
            t_join_block* join_block = (t_join_block*)block->block_structure;
            PID pid = join_block->blocking_thread->pid;
            TID tid = join_block->blocking_thread->tid;

            //Si el tipo de bloqueo es JOIN, y si el tid y el pid del hilo bloqueante coincide con el tid y
            // el pid del hilo pasado(hilo a liberar), lo retorno.
            return block->block_type == JOIN && thread->tid == tid && strcmp(thread->pid, pid) == 0;
        }
        t_block* block = (t_block*)list_remove_by_condition(BLOCKED, condition);

        //Mientras que siga encontrando bloqueos, sigo ejecutando, cuando no encuentre mas, corto las iteraciones
        if(block == NULL){

            execute = false;
        } else {

            t_join_block* join_block = (t_join_block*)block->block_structure;
            t_thread* blocked = join_block->blocked_thread;

            //Creo un nuevo intervalo y le asigno su tiempo de inicio para agregarlo a la lista de readys del hilo bloqueado
            t_interval* new_ready = new_interval();
            *(new_ready->start_time) = get_time();

            list_add(blocked->ready_list, (void*)new_ready);

            //Agrego el hilo a la lista de readys de su programa correspondiente
            list_add(program->ready, (void*)blocked);

            //Libero las estructuras del bloqueo
            free(block->block_structure);
            free(block);
        }
    }
}