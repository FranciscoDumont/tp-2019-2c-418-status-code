#include "suse.h"

SUSEConfig* config;
t_log* logger;
t_list* programs;
t_list* NEW;
t_list* BLOCKED;
t_list* EXIT;
t_list* asking_for_thread;
t_list* semaphores;

pthread_mutex_t mutex_logger;
pthread_mutex_t mutex_programs;

//--LISTO
int main() {
    pthread_t metrics_thread;
    void* metrics_thread_error;

    start_log();

    initialize_structures();

    read_config_options();

    initialize_semaphores();

    pthread_create(&metrics_thread, NULL, metrics_function, NULL);

    server_function();

    pthread_join(metrics_thread, &metrics_thread_error);

    return 0;
}

//--LISTO
void start_log(){
    logger = log_create("../suse.log", "suse", 1, LOG_LEVEL_INFO);
}

void initialize_structures(){
    pthread_mutex_init(&mutex_logger, NULL);
    pthread_mutex_init(&mutex_programs, NULL);

    config = malloc(sizeof(SUSEConfig));
    programs = list_create();
    NEW = list_create();
    BLOCKED = list_create();
    EXIT = list_create();
    asking_for_thread = list_create();
    semaphores = list_create();

    pthread_mutex_lock(&mutex_logger);
    log_trace(logger, "Structures initialized...");
    pthread_mutex_unlock(&mutex_logger);
}

void read_config_options(){
    t_config* config_file = config_create("../suse.config");
    config->listen_port = config_get_int_value(config_file, "LISTEN_PORT");
    config->metrics_timer = config_get_int_value(config_file, "METRICS_TIMER");
    config->max_multiprog = config_get_int_value(config_file, "MAX_MULTIPROG");
    config->sem_ids = config_get_array_value(config_file, "SEM_IDS");
    config->sem_init = config_get_array_value(config_file, "SEM_INIT");
    config->sem_max = config_get_array_value(config_file, "SEM_MAX");
    config->alpha_sjf = config_get_double_value(config_file, "ALPHA_SJF");
    log_trace(logger,
              "Config file read: LISTEN_PORT: %d, METRICS_TIMER: %d, MAX_MULTIPROG: %d, ALPHA_SJF: %f.",
              config->listen_port,
              config->metrics_timer,
              config->max_multiprog,
              config->alpha_sjf);
    config_destroy(config_file);
}

void initialize_semaphores(){

    int pos = 0;
    char** ptr = config->sem_ids;

    //Itero el array de semaforos
    for (char* id = *ptr; id; id=*++ptr) {

        //Mientras que el id sea distinto de null sigo
        if(id != NULL){

            //Transformo los valores de configuracion a enteros
            int max_value = atoi(config->sem_max[pos]);
            int init_value = atoi(config->sem_init[pos]);

            //Reservo memoria para un semaforo
            t_semaphore* semaphore = malloc(sizeof(t_semaphore));

            //Asigno valores al semaforo
            semaphore->id = id;
            semaphore->max_value = max_value;
            semaphore->current_value = init_value;
            semaphore->blocked_threads = list_create();

            //Agrego semaforos a su lista
            list_add(semaphores, (void*)semaphore);

            //Creo una nueva estructura de bloqueo y le agrego el tipo semaforo
            t_block* new_block = malloc(sizeof(t_block));
            new_block->block_type = SEMAPHORE;

            //Creo un nuevo bloqueo por semaforo y le asigno el semaforo
            t_semaphore_block* new_semaphore_block = malloc(sizeof(t_semaphore_block));
            new_semaphore_block->semaphore = semaphore;

            //Agrego el bloqueo por semaforo a la estructura del bloqueo
            new_block->block_structure = (void*)new_semaphore_block;

            //Agrego el bloqueo a la lista de bloqueos
            list_add(BLOCKED, (void*)new_block);

            pthread_mutex_lock(&mutex_logger);
            log_trace(logger, "Semaphore: %s added, max_value: %d, init_value: %d", id, max_value, init_value);
            pthread_mutex_unlock(&mutex_logger);

            pos++;
        }
    }
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
        log_trace(logger, "Program: %d has disconnected", fd);
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
            case SUSE_WAIT:
            {
                suse_wait(fd, ip, port, cosas);
                break;
            }
            case SUSE_SIGNAL:
            {
                suse_signal(fd, ip, port, cosas);
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
    PID pid = fd;
    t_program* new_program = (t_program*)malloc(sizeof(t_program));
    new_program->pid = fd;
    new_program->ready = list_create();
    new_program->exec = NULL;
    new_program->executing = false;

    pthread_mutex_lock(&mutex_programs);
    list_add(programs, (void*)new_program);
    pthread_mutex_unlock(&mutex_programs);

    pthread_mutex_lock(&mutex_logger);
    log_trace(logger, "New program added, PID:%d", pid);
    pthread_mutex_unlock(&mutex_logger);
}

//--LISTO
void suse_create(int fd, char * ip, int port, t_list* received){
    PID pid = fd;
    int tid = *((int*)list_get(received, 0));
    int return_code;

    //Creo un nuevo hilo
    t_thread* new_thread = malloc(sizeof(t_thread));
    new_thread->tid = tid;
    new_thread->pid = pid;
    new_thread->exec_list = list_create();
    new_thread->ready_list = list_create();
    new_thread->start_time = malloc(sizeof(struct timespec));
    *(new_thread->start_time) = get_time(); //Esto esta bien? o tendria que ser desde que se empieza a ejecutar o desde
    // que entra al programa?

    //Busco al programa en el que se agregaria el nuevo hilo
    t_program* program = find_program(pid);

    //Verifico si el nivel actual de planificacion es mayor al limite predefinido
    if(multiprogramming_grade() >= config->max_multiprog){

        //Agrego el hilo creado a new
        list_add(NEW, (void*)new_thread);

        //Si el programa ya posee hilos, retorno un 1 y el cliente sigue ejecutando
        if(program->executing){
            return_code = 1;

        //Si no posee, le devuelvo -1, y el cliente se cuelga esperando a que algun close le habilite algun lugar
        } else {
            return_code = -1;
        }
        pthread_mutex_lock(&mutex_logger);
        log_trace(logger, "Program(%d)'s Thread(%d) added to NEW list", pid, tid);
        pthread_mutex_unlock(&mutex_logger);

    } else {

        //Creo un nuevo intervalo
        t_interval* first_iteration = new_interval();
        *(first_iteration->start_time) = get_time();

        //Verifico si el programa ya entro a la planificacion o esta esperando lugar
        if(program->executing) {

            //Verifico si hay algun hilo en exec
            if(program->exec != NULL){

                //Agrego el hilo a la lista de listos del programa y agrego el intervalo a la lista de intervalos de
                // listo del hilo
                t_list *ready = program->ready;
                list_add(ready, new_thread);

                list_add(new_thread->ready_list, (void*)first_iteration);

                pthread_mutex_lock(&mutex_logger);
                log_trace(logger, "Thread(%d), added to Program(%d)'s ready list", tid, pid);
                pthread_mutex_unlock(&mutex_logger);

            //En este caso, el programa esta en planificacion, pero no hay ningun hilo en exec, puede ocurrir si se
            // llamo a close y todos los demas hilos estaban bloqueados
            } else {

                //Agrego un nuevo intervalo de ejecucion y luego el hilo al estado de exec de su programa correspondiente
                list_add(new_thread->exec_list, (void*)first_iteration);
                program->exec = new_thread;

                pthread_mutex_lock(&mutex_logger);
                log_trace(logger, "Program(%d)'s Thread(%d) added and executing", pid, tid);
                pthread_mutex_unlock(&mutex_logger);
            }

        //En este caso el programa no habia entrado a la planificacion por falta de lugar segun el nivel de
        // multiprogramacion
        } else {

            //Como no se esta ejecutando, agrego el intervalo creado a la lista de ejecucion del nuevo hilo, agrego el
            // mismo al parametro exec del programa y lo marco como ejecutandose
            list_add(new_thread->exec_list, (void*)first_iteration);
            program->exec = new_thread;
            program->executing = true;

            pthread_mutex_lock(&mutex_logger);
            log_trace(logger, "Program(%d)'s Thread(%d) added and executing", pid, tid);
            pthread_mutex_unlock(&mutex_logger);
        }
        return_code = 1;
    }

    //Confirmo la planificacion del hilo
    create_response_thread(fd, return_code, SUSE_CREATE);

    void element_destroyer(void* element){
        free(element);
    }
    free_list(received, element_destroyer);
}

void suse_schedule_next(int fd, char * ip, int port, t_list* received){
    PID pid = fd;
    t_program* program = find_program(pid);
    int return_tid;

    //Verifico que el programa exista(que no se haya cerrado con suse_close)
    if(program != NULL) {

        pthread_mutex_lock(&mutex_logger);
        log_trace(logger, "Program: %d, asking to schedule a new thread", pid);
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
}

int schedule_next(t_program* program){
    int return_tid;

    t_list* ready = program->ready;
    if(list_size(ready) > 0){

        //Obtengo el proximo hilo a ejecutar
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
            log_trace(logger, "Thread: %d, exchanged for Thread: %d on Program: %d", old_tid, new_tid, program->pid);
            pthread_mutex_unlock(&mutex_logger);

        //El campo exec estaba vacio, lo ocupo directamente
        } else {

            //Obtengo el ultimo elemento(interval) de la lista de readys del proximo hilo a ejecutar y le agrego el tiempo final
            t_interval* last_ready_NEWONE = last_ready(thread_to_execute);

            *(last_ready_NEWONE->end_time) = *(new_start->start_time);

            //Le asigno a exec el nuevo hilo a ejecutar
            program->exec = thread_to_execute;
        }

        pthread_mutex_lock(&mutex_logger);
        log_trace(logger, "Thread: %d, now executing on Program: %d", return_tid, program->pid);
        pthread_mutex_unlock(&mutex_logger);

    //La lista de ready esta vacia, debo devolver el mismo hilo o bloquear
    } else {

        //El programa ya tiene un elemento en ejecucion
        if(program->exec != NULL){

            //Proximo hilo a ejecutar
            t_thread* thread_to_execute = program->exec;

            //Ultimo intervalo ejecutado
            t_interval* last_execd = last_exec(thread_to_execute);

            //Creo un nuevo intervalo y le asigno su tiempo de inicio
            t_interval* new_exec_interval = new_interval();
            *(new_exec_interval->start_time) = get_time();

            //Le asigno un final al ultimo intervalo de ejecucion del hilo
            *(last_execd->end_time) = *(new_exec_interval->start_time);

            //Agrego el nuevo intervalo de ejecucion a la lista de ejecucion del hilo
            list_add(thread_to_execute->exec_list, (void*)new_exec_interval);

            //Como el hilo no cambio, retorno el mismo tid
            return_tid = program->exec->tid;

            pthread_mutex_lock(&mutex_logger);
            log_trace(logger, "Thread: %d, now executing on Program: %d", return_tid, program->pid);
            pthread_mutex_unlock(&mutex_logger);

            //Por que creo otro intervalo de ejecucion en vez de usar el mismo? Porque de esta manera es evidente que
            // el hilo trabajo en 2 rafagas consecutivas pero distintas, caso contrario se podria pensar que el hilo
            // hizo una rafaga larga en vez de dos cortas

        //La lista de listos esta vacia, y no hay hilo en ejecucion, debo decirle al cliente que se bloquee
        } else {

            //Agrego el programa a la lista de programas que estan esperando un hilo, van a ser los primeros en ser
            // atendidos luego de un suse_close por ya estar ejecutandose
            list_add(asking_for_thread, (void*)program);
            return_tid = -1;

            pthread_mutex_lock(&mutex_logger);
            log_trace(logger, "Program: %d had no Ready or Exec threads, blocked until a new one is available", program->pid);
            pthread_mutex_unlock(&mutex_logger);
        }

    }

    return return_tid;
}

//TODO:implementar SJF-E, actualmente es FIFO
t_thread* schedule_new_thread(t_program* program){
    return (t_thread*)list_remove(program->ready, 0);
}

void suse_join(int fd, char * ip, int port, t_list* received){
    PID pid = fd;
    int tid = *((int*)list_get(received, 0));
    t_program* program = find_program(pid);

    //Obtengo el hilo en ejecucion
    t_thread* executing_thread = program->exec;
    int executing_tid = executing_thread->tid;

    pthread_mutex_lock(&mutex_logger);
    log_trace(logger, "Program: %d, asking to join Thread: %d with Thread: %d", pid, executing_tid, tid);
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

    //El hilo bloqueante no estaba en EXIT, paso a bloquear el que esta en exec
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

    //Existe algun caso en el que pueda fallar?
    create_response_thread(fd, 1, SUSE_JOIN);

    void element_destroyer(void* element){
        free(element);
    }
    free_list(received, element_destroyer);

}

void suse_close(int fd, char * ip, int port, t_list* received){
    PID pid = fd;
    int tid = *((int*)list_get(received, 0));
    t_program* program = find_program(pid);
    int response;

    pthread_mutex_lock(&mutex_logger);
    log_trace(logger, "Program: %d, asking to finish thread: %d", pid, tid);
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

        pthread_mutex_lock(&mutex_logger);
        log_trace(logger, "Thread: %d, from Program: %d, sent to EXIT", tid, pid);
        pthread_mutex_unlock(&mutex_logger);

        //Verifico si no quedan mas hilos en ready o exec
        if(no_more_threads(program)){

            destroy_exit_threads(pid);

            //Destruyo el programa
            destroy_program(pid);
            pthread_mutex_lock(&mutex_logger);
            log_trace(logger, "Program: %d, exited SUSE", pid);
            pthread_mutex_unlock(&mutex_logger);
        }

        //Lanzo un hilo detacheable que genera metricas
        pthread_t end_thread_metrics_thread;
        pthread_create(&end_thread_metrics_thread, NULL, generate_metrics, NULL);
        pthread_detach(end_thread_metrics_thread);

        //Reparto los hilos que se encuentren en NEW a sus respectivas colas de ready segun el grado de multiprogramacion
        //SIEMPRE la cantidad de hilos a distribuir va a ser UNO, ya que se llama al close de a una vez por hilo, y los
        // hilos de exit NO cuentan para el grado de multiprogramacion(no importa cuando este liberando las estructuras
        // de los hilos)
        distribute_new_thread();

        response = 1;
    } else {

        //El hilo a cerrar no era el que estaba en ejecucion
        response = -1;
    }

    //1 para exito, -1 en el caso de error
    create_response_thread(fd, response, SUSE_CLOSE);

    void element_destroyer(void* element){
        free(element);
    }
    free_list(received, element_destroyer);
}

void distribute_new_thread(){

    //Busco algun hilo de algun programa(en NEW) que ya este en ejecucion
    bool condition(void* _thread){
        t_thread* thread = (t_thread*)_thread;

        //Verifico si hay algun programa que posea el mismo pid que el pid del hilo, como estos son los programas en
        // ejecucion, esto significa que este hilo pertenece a un programa en ejecucion
        bool second_condition(void* _program){

            t_program* program = (t_program*)_program;
            return program->pid == thread->pid && program->executing;
        }
        return list_any_satisfy(programs, &second_condition);
    }
    t_thread* next_thread = (t_thread*)list_remove_by_condition(NEW, condition);

    //verifico que el hilo obtenido no sea nulo
    if(next_thread != NULL){

        //Hallo el programa correspondiente al hilo hallado
        t_program* program = find_program(next_thread->pid);

        //Verificar si el programa se encuentra en la lista de programas que estan bloqueados esperando por un hilo
        if(is_in_asking_for_thread(program)){

            //Remover el programa de la lista de esperando
            remove_from_asking_for_thread(program);

            //Asigno el programa al estado exec del programa correspondiente, creo los intervalos y retorno por socket
            // el valor del tid
            assign_thread(program, next_thread, SUSE_SCHEDULE_NEXT);

        //El programa al que le voy a asignar el hilo, se encontraba en ejecucion, pero no estaba bloqueado esperando
        // un hilo, solo debo actualizar los intervalos y agregarlo a la lista de ready del mismo
        } else {

            //Creo un nuevo intervalo y le asigno un momento de inicio
            t_interval* new_ready = new_interval();
            *(new_ready->start_time) = get_time();

            //Agrego el intervalo a la lista de listos del hilo
            list_add(next_thread->ready_list, new_ready);

            //Agrego el hilo a la lista de listos del programa y agrego el intervalo a la lista de intervalos de
            // listo del hilo
            t_list *ready = program->ready;
            list_add(ready, next_thread);

            pthread_mutex_lock(&mutex_logger);
            log_trace(logger, "Thread(%d), added to Program(%d)'s ready list", next_thread->tid, program->pid);
            pthread_mutex_unlock(&mutex_logger);
        }

    //No habia hilos para programas en ejecucion
    } else {

        //Obtengo el primer hilo de la lista de NEW para repartir
        next_thread = (t_thread*)list_remove(NEW, 0);

        //Verifico que el hilo anterior no sea nulo
        if(next_thread != NULL){

            //Hallo el programa correspondiente al hilo hallado
            t_program* program = find_program(next_thread->pid);

            //En este caso el programa no pertenecia a la lista de programas pidiendo hilo, ni a la lista de programas
            // en ejecucion(pero que no habian pedido un hilo), por lo que el programa esta bloqueado en create,
            // ponerlo en ejecucion, asignarle el hilo en exec y actualizar los intervalos
            assign_thread(program, next_thread, SUSE_CREATE);
        }
    }
}

void assign_thread(t_program* program, t_thread* thread, MessageType header){

    int response;

    //Creo un nuevo intervalo
    t_interval* exec = new_interval();
    *(exec->start_time) = get_time();

    //Agrego el intervalo a la lista de ejecucion del hilo
    list_add(thread->exec_list, (void*)exec);

    //Agrego el hilo al estad exec del programa
    program->exec = thread;

    //Verifico el tipo de funcion que origino el envio
    if(header == SUSE_SCHEDULE_NEXT){

        //TID a retornar
        response = thread->tid;

        pthread_mutex_lock(&mutex_logger);
        log_trace(logger, "Thread: %d, now executing on Program: %d", thread->tid, program->pid);
        pthread_mutex_unlock(&mutex_logger);
    } else {

        //Codigo de verificacion para suse_create
        response = 1;
        program->executing = true;

        pthread_mutex_lock(&mutex_logger);
        log_trace(logger, "Program(%d)'s Thread(%d) added and executing", program->pid, thread->tid);
        pthread_mutex_unlock(&mutex_logger);
    }

    create_response_thread(program->pid, response, header);
}

void suse_wait(int fd, char * ip, int port, t_list* received){

    int response;
    char* id = (char*)list_get(received, 1);
    t_semaphore* semaphore = find_semaphore(id);

    PID pid = fd;
    int tid = *((int*)list_get(received, 0));

    //Busco al hilo y al programa del mismo que me llamaron a wait
    t_program* program = find_program(pid);
    t_thread* thread = find_thread(program, tid);

    pthread_mutex_lock(&mutex_logger);
    log_trace(logger, "Thread: %d of program: %d asking for a wait on semaphore: %s", tid, pid, id);
    pthread_mutex_unlock(&mutex_logger);

    //Le resto al semaforo
    semaphore->current_value--;

    //Si el valor del semaforo es menor a 0, bloquear hilo
    if(semaphore->current_value < 0){

        //Agrego el hilo a la lista de hilos bloqueados
        list_add(semaphore->blocked_threads, (void*)thread);

        //Obtengo el ultimo intervalo de ejecucion y le asigno su tiempo de finalizacion
        t_interval* last_execd = last_exec(thread);
        *(last_execd->end_time) = get_time();

        //Pongo el estado exec del programa en null
        program->exec = NULL;

        pthread_mutex_lock(&mutex_logger);
        log_trace(logger, "Thread: %d of program: %d blocked by a wait on semaphore: %s", tid, pid, id);
        pthread_mutex_unlock(&mutex_logger);
    }

    //Existe alguna posibilidad de error?
    response = 1;

    create_response_thread(fd, response, SUSE_WAIT);

    void element_destroyer(void* element){
        free(element);
    }
    free_list(received, element_destroyer);
}

void suse_signal(int fd, char * ip, int port, t_list* received){

    int response;
    char* id = (char*)list_get(received, 1);
    t_semaphore* semaphore = find_semaphore(id);

    PID pid = fd;
    int tid = *((int*)list_get(received, 0));

    //Busco al hilo y al programa del mismo que me llamaron a signal
    t_program* program = find_program(pid);
    t_thread* thread = find_thread(program, tid);

    pthread_mutex_lock(&mutex_logger);
    log_trace(logger, "Thread: %d of program: %d asking for a signal on semaphore: %s", tid, pid, id);
    pthread_mutex_unlock(&mutex_logger);

    //TODO:Verificar si tendria que pasar algo en caso de que el current_value exceda al max_value
    semaphore->current_value++;

    //Si el valor actual es mayor al maximo, lo reduzco hasta el valor maximo
    if(semaphore->current_value > semaphore->max_value){
        semaphore->current_value = semaphore->max_value;
    }

    //Si el avalor actual es menor o igual a 0, libero a uno de los hilos bloqueados
    if(semaphore->current_value <= 0){

        //Traigo el primer elemento de la lista para desbloquear
        t_thread* thread_to_unblock = (t_thread*)list_remove(semaphore->blocked_threads, 0);

        //Creo un nuevo intervalo para agregar a la lista de ready
        t_interval* new_ready = new_interval();
        *(new_ready->start_time) = get_time();
        list_add(thread_to_unblock->ready_list, (void*)new_ready);

        //Busco el programa al que pertenece el hilo
        t_program* _program = find_program(thread_to_unblock->pid);

        //Agrego el hilo a la lista de listos del programa padre
        list_add(_program->ready, (void*)thread_to_unblock);

        pthread_mutex_lock(&mutex_logger);
        log_trace(logger, "Thread: %d of program: %s asking unblocked by signal on semaphore: %s", thread_to_unblock->tid, thread_to_unblock->pid, id);
        pthread_mutex_unlock(&mutex_logger);
    }

    response = 1;

    //Existe algun caso de error?
    create_response_thread(fd, response, SUSE_SIGNAL);

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
    char* system_metrics = generate_system_metrics();
    string_append(&metric_to_log, system_metrics);
    free(system_metrics);
    string_append(&metric_to_log, separator);
    char* program_metrics = generate_program_metrics();
    string_append(&metric_to_log, program_metrics);
    free(program_metrics);
    string_append(&metric_to_log, separator);
    char* thread_metrics = generate_thread_metrics();
    string_append(&metric_to_log, thread_metrics);
    free(thread_metrics);
    string_append(&metric_to_log, separator);

    pthread_mutex_lock(&mutex_logger);
    log_info(logger, metric_to_log);
    pthread_mutex_unlock(&mutex_logger);

    free(metric_to_log);
}

char* generate_thread_metrics(){
    char* metrics = string_new();
    string_append(&metrics, "Thread metrics:\n");
    return metrics;
}

char* generate_program_metrics(){

    char* metrics = string_new();
    string_append(&metrics, "Program metrics:\n");

    if(list_size(programs) != 0){
        void iterate(void* _program){
            t_program* program = (t_program*)_program;
            char* separator = "\n";
            string_append(&metrics, "--Program: ");
            char* pid = string_itoa(program->pid);
            string_append(&metrics, pid);
            free(pid);
            string_append(&metrics, separator);
            string_append(&metrics, "----Threads in NEW: ");
            char* new = string_itoa(threads_in_new(program));
            string_append(&metrics, new);
            free(new);
            string_append(&metrics, separator);
            string_append(&metrics, "----Threads in READY: ");
            char* ready= string_itoa(threads_in_ready(program));
            string_append(&metrics, ready);
            free(ready);
            string_append(&metrics, separator);
            string_append(&metrics, "----Threads in RUN: ");
            char* run = string_itoa(threads_in_exec(program));
            string_append(&metrics, run);
            free(run);
            string_append(&metrics, separator);
            string_append(&metrics, "----Threads in BLOCKED: ");
            char* blocked = string_itoa(threads_in_blocked(program));
            string_append(&metrics, blocked);
            free(blocked);
            string_append(&metrics, separator);
        }
        list_iterate(programs, iterate);
    } else {
        string_append(&metrics, "--No more programs in scheduler\n");
    }


    return metrics;
}

char* generate_system_metrics(){
    char* metrics = string_new();
    char* separator = "\n";
    char* sml = "\nSystem metrics:\n";
    string_append(&metrics, sml);
    char* mgl = "--Multiprogramming grade: ";
    string_append(&metrics, mgl);
    char* mg = string_itoa(multiprogramming_grade());
    string_append(&metrics, mg);
    free(mg);
    string_append(&metrics, separator);
    char* sm = generate_semaphore_metrics();
    string_append(&metrics, sm);
    free(sm);
    return metrics;
}

char* generate_semaphore_metrics(){
    char* metrics = string_new();
    string_append(&metrics, "--Semaphores:\n");

    void iterate(void* _semaphore){
        t_semaphore* semaphore = (t_semaphore*)_semaphore;
        char* separator = "\n";
        string_append(&metrics, "----Semaphore: ");
        string_append(&metrics, semaphore->id);
        string_append(&metrics, separator);
        string_append(&metrics, "------Current value: ");
        char* new = string_itoa(semaphore->current_value);
        string_append(&metrics, new);
        free(new);
        string_append(&metrics, separator);
    }
    list_iterate(semaphores, iterate);

    return metrics;
}

//--Helpers

int multiprogramming_grade(){

    //Busco la cantidad de hilos en BLOCKED de cada programa y la sumo con fold
    void* seedB = malloc(sizeof(int));
    *((int*)seedB) = 0;

    void* seedB_plus_grade(void* _seedB, void* program) {
        *((int*) _seedB) += threads_in_blocked((t_program*)program);
        return _seedB;
    }

    void* blocked_grade_ptr = list_fold(programs, seedB, &seedB_plus_grade);
    int blocked_grade = *((int*)blocked_grade_ptr);
    free(blocked_grade_ptr);

    //Busco la cantidad de hilos en READY de cada programa y la sumo con fold
    void* seed = malloc(sizeof(int));
    *((int*)seed) = 0;

    void* seed_plus_grade(void* seed, void* program){
        *((int*) seed) += threads_in_ready((t_program*)program);
        return seed;
    }

    void* ready_grade_ptr = list_fold(programs, seed, &seed_plus_grade);
    int ready_grade = *((int*)ready_grade_ptr);
    free(ready_grade_ptr);

    //Busco los programas con un hilo en ejecucion y cuento la lista resultante
    bool executing_program(void* program){
        return ((t_program*)program)->exec != NULL;
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
        return ((t_program*)program)->pid == pid;
    }
    return (t_program*)list_find(programs, &program_finder);
}

t_thread* find_thread(t_program* program, TID tid){
    t_thread* thread = NULL;
    PID pid = program->pid;

    //Buscador de hilos solo por tid
    bool tid_thread_finder(void* _thread){
        return ((t_thread*)_thread)->tid == tid;
    }

    //Buscador de hilos por tid y pid
    bool tid_pid_thread_finder(void* _thread){
        return ((t_thread*)_thread)->tid == tid && ((t_thread*)_thread)->pid == program->pid;
    }

    //Busco el hilo en la lista de ready del programa al que pertenece
    thread = (t_thread*)list_find(program->ready, tid_thread_finder);

    //Si el hilo no se encuentra en la lista de readys del programa, busco en EXIT
    if(thread == NULL){
        //Busco el hilo en EXIT
        thread = (t_thread*)list_find(EXIT, tid_pid_thread_finder);

        //Si el hilo no se encuentra en la lista de EXIT, busco en NEW
        if(thread == NULL){
            //Busco el hilo en NEW
            thread = (t_thread*)list_find(NEW, tid_pid_thread_finder);

            //TODO:verificar que no sea NULL y si lo es, buscar en la lista de BLOCKED
        }
    }

    return thread;
}

t_semaphore* find_semaphore(char* id){
    bool semaphore_finder(void* semaphore){
        return strcmp(((t_semaphore*)semaphore)->id, id) == 0;
    }
    return (t_semaphore*)list_find(semaphores, &semaphore_finder);
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
    free(thread);
}

bool no_more_threads(t_program* program){
    return (threads_in_new(program) + threads_in_ready(program) + threads_in_blocked(program) + threads_in_exec(program)) == 0;
}

int threads_in_new(t_program* program){
    bool condition(void* _thread){
        t_thread* thread = (t_thread*)_thread;
        return program->pid == thread->pid;
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

        if(block->block_type == JOIN){
            t_join_block* join_block = (t_join_block*)(block->block_structure);
            t_thread* blocked_thread = join_block->blocked_thread;
            return blocked_thread->pid == program->pid;
        } else {
            return false;
        }

    }
    return list_count_satisfying(BLOCKED, condition);
}

int threads_in_semaphore_block(t_program* program){

    void* seedB = malloc(sizeof(int));
    *((int*)seedB) = 0;

    void* seedB_plus_grade(void* _seedB, void* _block) {
        t_block* block = (t_block*)_block;
        if(block->block_type == SEMAPHORE){

            t_semaphore_block* s_block = (t_semaphore_block*)block->block_structure;
            t_semaphore* semaphore = s_block->semaphore;

            bool condition(void* _thread){
                t_thread* thread = (t_thread*)_thread;
                return thread->pid == program->pid;
            }
            *((int*) _seedB) = list_count_satisfying(semaphore->blocked_threads, condition);

        } else {
            *((int*) _seedB) = 0;
        }

        return _seedB;
    }

    void* blocked_grade_ptr = list_fold(BLOCKED, seedB, &seedB_plus_grade);
    int blocked_grade = *((int*)blocked_grade_ptr);
    free(blocked_grade_ptr);

    return blocked_grade;
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
        return pid == program->pid;
    }
    void element_destroyer(void* _program){
        t_program* program = (t_program*)_program;
        list_destroy(program->ready);
        free(program);
    }
    list_remove_and_destroy_by_condition(programs, condition, element_destroyer);
}

void destroy_exit_threads(PID pid){
    bool execute = true;
    while(execute){
        bool condition(void* _thread){
            t_thread* thread1 = (t_thread*)_thread;
            return thread1->pid == pid;
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
        return thread_to_compare->tid == thread->tid && thread_to_compare->pid == thread->pid;
    }
    return list_any_satisfy(EXIT, condition);
}

void free_join_blocks(t_thread* thread, t_program* program){
    bool execute = true;

    //Mientras que siga habiendo elementos, sigo iterando sobre la lista de BLOCKED
    while(execute){
        bool condition(void* _block){
            t_block* block = (t_block*)_block;
            if(block->block_type == JOIN){
                t_join_block* join_block = (t_join_block*)block->block_structure;

                //TID y PID del hilo bloqueante
                PID pid = join_block->blocking_thread->pid;
                TID tid = join_block->blocking_thread->tid;

                //Si el tid y el pid del hilo bloqueante coincide con el tid y
                // el pid del hilo pasado(hilo a liberar), lo retorno.
                return thread->tid == tid && thread->pid == pid;
            } else {
                return false;
            }
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

bool is_in_asking_for_thread(t_program* program){

    bool condition(void* _program){

        return ((t_program*)_program)->pid == program->pid;
    }
    return list_any_satisfy(asking_for_thread, condition);
}

void remove_from_asking_for_thread(t_program* program){

    bool condition(void* _program){

        return ((t_program*)_program)->pid == program->pid;
    }
    list_remove_by_condition(asking_for_thread, condition);
}