#include "libSuse.h"

int max_tid = 0;
bool server_socket_initialized = false;
int server_socket = 0;
libSUSEConfig* config;

//--LISTO
void suse_init(){
    config = malloc(sizeof(libSUSEConfig));
    config->ip = malloc(sizeof(char));
    read_config_options();
    if((server_socket = create_socket()) == -1) {
        printf("Error creating socket\n");
        return;
    }
    if(-1 == connect_socket(server_socket, config->ip, config->talk_port)){
        printf("Error connecting to SUSE server\n");
        return;
    }

    server_socket_initialized = true;
}

//--LISTO
void read_config_options(){
    t_config* config_file = config_create("../libsuse.config");
    config->ip = strcpy(config->ip, config_get_string_value(config_file, "IP"));
    config->talk_port = config_get_int_value(config_file, "TALK_PORT");
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
            printf("Error sending a new thread to planning server\n");
            return -1;
        } else {
            free(_tid);
            free_package(package);
            if(confirm_action() == 1){
                printf("Hilo en planificacion\n");
                return 0;
            } else {
                printf("Failed receiving closing new thread confirmation\n");
                return -1;
            }
        }
    }
}

//--TODO:Agregar un recv bloqueante si no hay mas hilos en SUSE?
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
        printf("Failed asking for new scheduled thread\n");
        return -1;
    } else {
        free(placebo);
        free_package(package);
        int new_scheduled_thread = confirm_action();
        if(new_scheduled_thread >= 0){
            printf("Scheduling next thread %i...\n", new_scheduled_thread);
            return new_scheduled_thread;
        } else {
            printf("Failed receiving scheduled thread\n");
            return -1;
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
        printf("Error sending thread: %d to block\n", tid_to_block);
        return -1;
    } else {
        free(_tid);
        free_package(package);
        if(confirm_action() == 1){
            printf("Blocked thread %i\n", tid_to_block);
            return 0;
        } else {
            printf("Failed receiving blocking thread confirmation\n");
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
        printf("Error sending thread: %d to close\n", tid);
        return -1;
    } else {
        free(_tid);
        free_package(package);
        if(confirm_action() == 1){
            printf("Closed thread %i\n", tid);
            return 0;
        } else {
            printf("Failed receiving closing thread confirmation\n");
            return -1;
        }
    }
}

int suse_wait(int tid, char *sem_name){
    // Not supported
    return 0;
}

int suse_signal(int tid, char *sem_name){
    // Not supported
    return 0;
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
    void element_destroyer(void* element){
        free(element);
    }

    MessageHeader* buffer_header = malloc(sizeof(MessageHeader));
    if(-1 == receive_header(server_socket, buffer_header)){
        return -1;
    }
    t_list *cosas = receive_package(server_socket, buffer_header);

    int rta = *((int*)list_get(cosas, 0));
    //list_destroy_and_destroy_elements(cosas, element_destroyer);
    return rta;
}