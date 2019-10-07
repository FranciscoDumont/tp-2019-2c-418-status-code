#include "suse.h"

SUSEConfig config;
t_log* logger;
t_list* NEW;
t_list* BLOCKED;
t_list* EXIT;
t_list* programs;
pthread_mutex_t* mutex_logger;

int main() {

    void* server_thread_error;
    void* metrics_thread_error;
    pthread_t server_thread;
    pthread_t metrics_thread;

    start_log();

    read_config_options();

    initialize_structures();

    mutex_logger = malloc(sizeof(pthread_mutex_t*));
    pthread_mutex_init(mutex_logger, NULL);
    pthread_create(&server_thread, NULL, server_function, NULL);
    pthread_create(&metrics_thread, NULL, metrics_function, NULL);

    pthread_join(server_thread, &server_thread_error);
    pthread_join(metrics_thread, &metrics_thread_error);

    //TODO:Loggear el error del servidor
    //int server_error = (int) (intptr_t) thread_server_error;

    return 0;
}

void start_log(){
    logger = log_create("../suse.log", "suse", 1, LOG_LEVEL_INFO);
}

void read_config_options(){
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

void initialize_structures(){
    NEW = list_create();
    BLOCKED = list_create();
    EXIT = list_create();
    programs = list_create();
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
        //TODO:crear las estructuras correspondientes al proceso?
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

void* metrics_function(void* arg){
    while(1){
        sleep(config.metrics_timer);
        generate_metrics();
    }
}

void generate_metrics(){
    char* metric_to_log = string_new();
    char* separator = "\n";
    string_append(&metric_to_log, separator);
    char* thread_metrics = generate_thread_metrics();
    string_append(&metric_to_log, thread_metrics);
    string_append(&metric_to_log, separator);
    char* program_metrics = generate_program_metrics();
    string_append(&metric_to_log, program_metrics);
    string_append(&metric_to_log, separator);
    char* system_metrics = generate_system_metrics();
    string_append(&metric_to_log, system_metrics);
    string_append(&metric_to_log, separator);

    pthread_mutex_lock(mutex_logger);
    log_info(logger, metric_to_log);
    pthread_mutex_unlock(mutex_logger);

    free(metric_to_log);
}

char* generate_thread_metrics(){}

char* generate_program_metrics(){}

char* generate_system_metrics(){}

void suse_init(int fd, char * ip, int port, MessageHeader * headerStruct){}

void suse_create(int fd, char * ip, int port, MessageHeader * headerStruct){}

void suse_schedule_next(int fd, char * ip, int port, MessageHeader * headerStruct){}

void suse_wait(int fd, char * ip, int port, MessageHeader * headerStruct){}

void suse_signal(int fd, char * ip, int port, MessageHeader * headerStruct){}

void suse_join(int fd, char * ip, int port, MessageHeader * headerStruct){}