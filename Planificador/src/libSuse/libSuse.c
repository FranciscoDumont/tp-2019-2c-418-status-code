#include "libSuse.h"

int max_tid = 0;
bool server_socket_initialized = false;
int server_socket = 0;
libSUSEConfig* config;

void suse_init(){
    config = (libSUSEConfig*)malloc(sizeof(libSUSEConfig*));
    config->ip = malloc(sizeof(char*));
    read_config_options();
    if((server_socket = create_socket()) == -1) {
        printf("Error al crear el socket\n");
        return;
    }
    if(-1 == connect_socket(server_socket, config->ip, config->talk_port)){
        printf("Error al conectarse al servidor\n");
        return;
    }

    server_socket_initialized = true;
}

void read_config_options(){
    t_config* config_file = config_create("../libsuse.config");
    config->ip = strcpy(config->ip, config_get_string_value(config_file, "IP"));
    config->talk_port = config_get_int_value(config_file, "TALK_PORT");
    config_destroy(config_file);
}

int suse_create(int tid){
    //--Si la conexion no esta inicializada la inicializo
    if(!server_socket_initialized){
        suse_init();
    }
    //--Si la conexion no esta inicializada ocurrio algun problema durante la inicializacion, informar, salir
    if(!server_socket_initialized){
        //TODO:retornar?
        return -1;
    } else {
        printf("TID a planificar: %d\n", tid);

        t_paquete *package = create_package(SUSE_CREATE);
        void* _tid = malloc(sizeof(int));
        *((int*)_tid) = tid;
        add_to_package(package, _tid, sizeof(int) + 1);
        if(send_package(package, server_socket) == -1){
            printf("Error en el envio...\n");
        } else {
            printf("Mensaje enviado\n");
        }

        if(confirm_action()){
            printf("Hilo en planificacion\n");
        }

        if (tid > max_tid) max_tid = tid;
        return 0;
    }
}

int suse_schedule_next(void){
    int next = max_tid;
    printf("Scheduling next item %i...\n", next);
    return next;
}

int suse_join(int tid){
    // Not supported
    return 0;
}

int suse_close(int tid){
    printf("Closed thread %i\n", tid);
    max_tid--;
    return 0;
}

int suse_wait(int tid){
    // Not supported
    return 0;
}

int suse_signal(int tid){
    // Not supported
    return 0;
}

int suse_return(int tid){
    // Not supported
    return 0;
}

static struct hilolay_operations hiloops = {
        .suse_create = &suse_create,
        .suse_schedule_next = &suse_schedule_next,
        .suse_join = &suse_join,
        .suse_close = &suse_close
};

void hilolay_init(void){
    init_internal(&hiloops);
}

bool confirm_action(){
    MessageHeader* buffer_header = malloc(sizeof(MessageHeader));
    if(-1 == receive_header(server_socket, buffer_header)){
        return false;
    }
    t_list *cosas = receive_package(server_socket, buffer_header);

    int rta = *((int*)list_get(cosas, 0));
    return (bool)rta;
}