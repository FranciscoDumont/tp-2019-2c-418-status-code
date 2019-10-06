#include "suse.h"

SUSEConfig config;
t_log* logger;

int main() {

    void* thread_server_error;
    pthread_t server_thread;

    logger = iniciar_log();

    leerConfig();

    pthread_create(&server_thread, NULL, server_function, NULL);

    pthread_join(server_thread, &thread_server_error);

    //TODO:Loguear el error del servidor
    //int server_error = (int) (intptr_t) thread_server_error;

    return 0;
}

t_log* iniciar_log(){
    return log_create("../suse.log", "suse", 1, LOG_LEVEL_INFO);
}

void leerConfig(){
    t_config* config_file = config_create("../suse.config");
    config.listen_port = config_get_int_value(config_file, "LISTEN_PORT");
    config.metrics_timer = config_get_int_value(config_file, "METRICS_TIMER");
    config.max_multiprog = config_get_int_value(config_file, "MAX_MULTIPROG");
    config.sem_ids = config_get_array_value(config_file, "SEM_IDS");
    config.sem_init = config_get_array_value(config_file, "SEM_INIT");
    config.sem_max = config_get_array_value(config_file, "SEM_MAX");
    config.alpha_sjf = config_get_double_value(config_file, "ALPHA_SJF");
    log_info(logger,
        "Configuración levantada: LISTEN_PORT: %d, METRICS_TIMER: %d, MAX_MULTIPROG: %d, ALPHA_SJF: %f.",
        config.listen_port,
        config.metrics_timer,
        config.max_multiprog,
        config.alpha_sjf);
    config_destroy(config_file);
}

void* server_function(void * arg){
    int PORT = config.listen_port;
    int socket;
    if((socket = create_socket()) == -1) {
        printf("Error al crear el socket");
        return (void *) -1;
    }
    if((bind_socket(socket, PORT)) == -1) {
        printf("Error al bindear el socket");
        return (void *) -2;
    }
    void new(int fd, char * ip, int port){
        printf("Cliente conectado, IP:%s, PORT:%d\n", ip, port);
    }

    void lost(int fd, char * ip, int port){
        printf("El cliente conectado en IP:%s, PORT:%d, ha muerto...\n", ip, port);
    }
    void incoming(int fd, char * ip, int port, MessageHeader * headerStruct){
        printf("Esperando mensaje...\n");

        t_list *cosas = receive_package(fd, headerStruct);

        switch (headerStruct->type){
            //Este caso esta solo de ejemplo, TODO:voletear
            case ABC:
            {
                ;
                char *mensaje = (char*)list_get(cosas, 0);
                printf("Mensaje recibido:%s\n", mensaje);
                t_paquete *package = create_package(ABC);
                add_to_package(package, (void*) mensaje, strlen(mensaje) + 1);
                if(send_package(package, fd) == -1){
                    printf("Error en el envio...\n");
                } else {
                    printf("Mensaje enviado\n");
                }
                break;
            }
            case SUSE_INIT:
            {
                printf("Iniciar\n");
                suse_init(fd, ip, port, headerStruct);
                break;
            }
            case SUSE_CREATE:
            {
                printf("Crear\n");
                suse_create(fd, ip, port, headerStruct);
                break;
            }
            case SUSE_SCHEDULE_NEXT:
            {
                printf("Schedulear\n");
                suse_schedule_next(fd, ip, port, headerStruct);
                break;
            }
            case SUSE_WAIT:
            {
                printf("Esperar\n");
                suse_wait(fd, ip, port, headerStruct);
                break;
            }
            case SUSE_SIGNAL:
            {
                printf("Liberar\n");
                suse_signal(fd, ip, port, headerStruct);
                break;
            }
            case SUSE_JOIN:
            {
                printf("Unir\n");
                suse_join(fd, ip, port, headerStruct);
                break;
            }
            default:
            {
                printf("Operacion desconocida. No quieras meter la pata\n");
                break;
            }
        }

    }
    start_server(socket, &new, &lost, &incoming);
}

void suse_init(int fd, char * ip, int port, MessageHeader * headerStruct){}

void suse_create(int fd, char * ip, int port, MessageHeader * headerStruct){}

void suse_schedule_next(int fd, char * ip, int port, MessageHeader * headerStruct){}

void suse_wait(int fd, char * ip, int port, MessageHeader * headerStruct){}

void suse_signal(int fd, char * ip, int port, MessageHeader * headerStruct){}

void suse_join(int fd, char * ip, int port, MessageHeader * headerStruct){}