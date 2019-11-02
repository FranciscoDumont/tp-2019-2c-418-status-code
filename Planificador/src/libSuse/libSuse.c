#include "libSuse.h"

bool server_socket_initialized = false;
int server_socket = 0;
libSUSEConfig* config;
t_log* logger;

//--LISTO
void suse_init(){
    start_log();
    config = malloc(sizeof(libSUSEConfig));
    config->ip = malloc(sizeof(char));
    read_config_options();

    if((server_socket = create_socket()) == -1) {
        log_error(logger, "Error creating socket");
        return;
    }
    if(-1 == connect_socket(server_socket, config->ip, config->talk_port)){
        log_error(logger, "Error connecting to SUSE server");
        return;
    }

    server_socket_initialized = true;
}

void start_log(){
    logger = log_create("../libSuse.log", "libSuse", 1, LOG_LEVEL_TRACE);
}

//--LISTO
void read_config_options(){
    t_config* config_file = config_create("../libsuse.config");
    config->ip = strcpy(config->ip, config_get_string_value(config_file, "IP"));
    config->talk_port = config_get_int_value(config_file, "TALK_PORT");
    log_trace(logger, "Config file read: TALK_PORT: %d, IP: %s.", config->talk_port, config->ip);
    config_destroy(config_file);
}

//--LISTO
int suse_create(int tid){
    //--Si la conexion no esta inicializada la inicializo
    if(!server_socket_initialized){
        suse_init();
    }
    //--Si la conexion no esta inicializada ocurrio algun problema durante la inicializacion, informar, salir
    if(!server_socket_initialized){
        return -1;
    } else {
        t_paquete *package = create_package(SUSE_CREATE);
        void* _tid = malloc(sizeof(int));
        *((int*)_tid) = tid;
        add_to_package(package, _tid, sizeof(int));
        if(send_package(package, server_socket) == -1){
            free(_tid);
            free_package(package);
            log_error(logger, "Error sending a new thread to planning server.");
            return -1;
        } else {
            free(_tid);
            free_package(package);
            if(confirm_action() == 1){
                log_trace(logger, "Hilo en planificacion.");
                return 0;
            } else {
                //Si la respuesta es distinta de 1, significa que no hay mas lugar en el planificador para mas hilos
                // por el nivel de multiprogramacion, y que el programa no estaba en ejecucion, por consiguiente, se
                // debe bloquear al cliente para evitar que continue con la ejecucion del hilo main, hasta que se libere
                // algun hilo de otro programa.
                log_trace(logger, "No hay mas lugar en el planificador, debera esperar a que se libere algun hilo SC.");
                if(confirm_action() == 1){
                    log_trace(logger, "Hilo en planificacion.");
                    return 0;
                } else {
                    log_error(logger, "Failed receiving closing new thread confirmation.");
                    return -1;
                }
            }
        }
    }
}

int suse_schedule_next(void){
    t_paquete *package = create_package(SUSE_SCHEDULE_NEXT);
    //Para hacer el servidor lo mas generico posible, todas las peticiones deben enviar algun dato,
    // pero en este caso(y otros probablemente) no es necesario enviar nada, por lo que envio un dato inutil para
    // evitar que el servidor se cuelgue esperando.
    void* placebo = malloc(sizeof(int));
    *((int*)placebo) = 1;
    add_to_package(package, placebo, sizeof(int));
    if(send_package(package, server_socket) == -1){
        free(placebo);
        free_package(package);
        log_error(logger, "Failed asking for new scheduled thread.");
        return -1;
    } else {
        free(placebo);
        free_package(package);
        int new_scheduled_thread = confirm_action();
        if(new_scheduled_thread >= 0){
            log_trace(logger, "Scheduling next thread %i...", new_scheduled_thread);
            return new_scheduled_thread;
        } else {
            //Si la respuesta es menor a 0 significa que todos sus hilos estan en NEW, por lo que se debe quedar
            // esperando(bloqueado) hasta que algun hilo de otro programa termine para que le habilite alguno a este
            // segun el grado de multiprogramacion
            log_trace(logger, "No hay mas lugar en el planificador, debera esperar a que se libere algun hilo SSN.");
            int new_scheduled_thread = confirm_action();
            if(new_scheduled_thread >= 0){
                log_trace(logger, "Volvimos del bloqueo de SSN.");
                log_trace(logger, "Scheduling next thread %i...", new_scheduled_thread);
                return new_scheduled_thread;
            } else {
                log_error(logger, "Failed receiving scheduled thread.");
                return -1;
            }
        }
    }
}

int suse_join(int tid){

    int tid_to_block = hilolay_get_tid();
    t_paquete *package = create_package(SUSE_JOIN);
    void* _tid = malloc(sizeof(int));
    *((int*)_tid) = tid;
    add_to_package(package, _tid, sizeof(int));
    if(send_package(package, server_socket) == -1){
        free(_tid);
        free_package(package);
        log_error(logger, "Error sending thread: %d to block", tid_to_block);
        return -1;
    } else {
        free(_tid);
        free_package(package);
        if(confirm_action() == 1){
            log_trace(logger, "Blocked thread %i", tid_to_block);
            return 0;
        } else {
            log_error(logger, "Failed receiving blocking thread confirmation.");
            return -1;
        }
    }
}

int suse_close(int tid){

    t_paquete *package = create_package(SUSE_CLOSE);
    void* _tid = malloc(sizeof(int));
    *((int*)_tid) = tid;
    add_to_package(package, _tid, sizeof(int));
    if(send_package(package, server_socket) == -1){
        free(_tid);
        free_package(package);
        log_error(logger, "Error sending thread: %d to close", tid);
        return -1;
    } else {
        free(_tid);
        free_package(package);
        if(confirm_action() == 1){
            log_trace(logger, "Closed thread %i", tid);
            return 0;
        } else {
            log_error(logger, "Failed receiving closing thread confirmation.");
            return -1;
        }
    }
}

int suse_wait(int tid, char *sem_name){

    t_paquete *package = create_package(SUSE_WAIT);
    void* _tid = malloc(sizeof(int));
    *((int*)_tid) = tid;
    add_to_package(package, _tid, sizeof(int));
    add_to_package(package, (void*)sem_name, strlen(sem_name) + 1);
    if(send_package(package, server_socket) == -1){
        free(_tid);
        free_package(package);
        log_error(logger, "Error sending wait: %d to block.", tid);
        return -1;
    } else {
        free(_tid);
        free_package(package);
        if(confirm_action() == 1){
            log_trace(logger, "Thread: %i on wait.", tid);
            return 0;
        } else {
            log_error(logger, "Failed receiving wait confirmation.");
            return -1;
        }
    }
}

int suse_signal(int tid, char *sem_name){

    t_paquete *package = create_package(SUSE_SIGNAL);
    void* _tid = malloc(sizeof(int));
    *((int*)_tid) = tid;
    add_to_package(package, _tid, sizeof(int));
    add_to_package(package, (void*)sem_name, strlen(sem_name) + 1);
    if(send_package(package, server_socket) == -1){
        free(_tid);
        free_package(package);
        log_error(logger, "Error sending signal: %d to block.", tid);
        return -1;
    } else {
        free(_tid);
        free_package(package);
        if(confirm_action() == 1){
            log_trace(logger, "Thread: %i signaled.", tid);
            return 0;
        } else {
            log_error(logger, "Failed receiving signal confirmation.", tid);
            return -1;
        }
    }
}

static struct hilolay_operations hiloops = {
        .suse_create = &suse_create,
        .suse_schedule_next = &suse_schedule_next,
        .suse_join = &suse_join,
        .suse_close = &suse_close,
        .suse_wait = &suse_wait,
        .suse_signal = &suse_signal
};

void hilolay_init(void){
    init_internal(&hiloops);
}

int confirm_action(){

    MessageHeader* buffer_header = malloc(sizeof(MessageHeader));
    if(-1 == receive_header(server_socket, buffer_header)){
        return -1;
    }
    t_list *cosas = receive_package(server_socket, buffer_header);

    int rta = *((int*)list_get(cosas, 0));
    return rta;
}