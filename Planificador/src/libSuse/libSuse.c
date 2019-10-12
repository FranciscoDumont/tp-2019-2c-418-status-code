#include "libSuse.h"

int max_tid = 0;
bool server_socket_initialized = false;
int server_socket = 0;
libSUSEConfig* config;

void suse_init(){
    config = malloc(sizeof(libSUSEConfig));
    config->ip = malloc(sizeof(char));
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
        return -1;
    } else {
        t_paquete *package = create_package(SUSE_CREATE);
        void* _tid = malloc(sizeof(int));
        *((int*)_tid) = tid;
        add_to_package(package, _tid, sizeof(int));
        if(send_package(package, server_socket) == -1){
            free(_tid);
            free_package(package);
            return -1;
        } else {
            free(_tid);
            free_package(package);
            if(confirm_action() == 1){
                printf("Hilo en planificacion\n");
                //TODO: averiguar si dejar esto aca
                if (tid > max_tid) max_tid = tid;
                return 0;
            } else {
                return -1;
            }
        }
    }
}

int suse_schedule_next(void){
    t_paquete *package = create_package(SUSE_SCHEDULE_NEXT);
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