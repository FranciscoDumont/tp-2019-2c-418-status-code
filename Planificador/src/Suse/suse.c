#include "suse.h"

SUSEConfig* config;
t_log* logger;
//Aplica a los 3, dictionary(tid-hilo)
t_list* NEW;
t_list* BLOCKED;
t_list* EXIT;
t_dictionary* programs;
pthread_mutex_t mutex_logger;
pthread_mutex_t mutex_programs;

int main() {

    void* server_thread_error;
    void* metrics_thread_error;
    pthread_t server_thread;
    pthread_t metrics_thread;

    start_log();

    initialize_structures();

    read_config_options();

    pthread_create(&server_thread, NULL, server_function, NULL);
    pthread_create(&metrics_thread, NULL, metrics_function, NULL);

    pthread_join(server_thread, &server_thread_error);
    pthread_join(metrics_thread, &metrics_thread_error);

    //TODO:Loggear el error del servidor
    //int server_error = (int) (intptr_t) thread_server_error;

    return 0;
}

void start_log(){
    logger = log_create("../suse.log", "suse", 1, LOG_LEVEL_TRACE);
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
        "Configuración levantada: LISTEN_PORT: %d, METRICS_TIMER: %d, MAX_MULTIPROG: %d, ALPHA_SJF: %f.",
        config->listen_port,
        config->metrics_timer,
        config->max_multiprog,
        config->alpha_sjf);
    config_destroy(config_file);
}

void initialize_structures(){
    pthread_mutex_init(&mutex_logger, NULL);
    pthread_mutex_init(&mutex_programs, NULL);
    config = (SUSEConfig*)malloc(sizeof(SUSEConfig*));
    config->sem_max = malloc(sizeof(char*));
    config->sem_init = malloc(sizeof(char*));
    config->sem_max = malloc(sizeof(char*));
    NEW = list_create();
    BLOCKED = list_create();
    EXIT = list_create();
    programs = dictionary_create();
    pthread_mutex_lock(&mutex_logger);
    log_trace(logger, "Estructuras inicializadas...");
    pthread_mutex_unlock(&mutex_logger);
}

void* server_function(void * arg){
    int PORT = config->listen_port;
    int socket;
    if((socket = create_socket()) == -1) {
        printf("Error al crear el socket");
        return (void *) -1;
    }
    if((bind_socket(socket, PORT)) == -1) {
        printf("Error al bindear el socket");
        return (void *) -2;
    }

    //--Funcion que se ejecuta cuando se conecta un nuevo programa
    void new(int fd, char * ip, int port){
        pthread_mutex_lock(&mutex_logger);
        log_trace(logger, "Se conecto un nuevo programa, IP:%s, PORT:%d", ip, port);
        pthread_mutex_unlock(&mutex_logger);
        t_newProgramData* newProgram = malloc(sizeof(t_newProgramData*));
        newProgram->ip = malloc(sizeof(char*));
        newProgram->ip = ip;
        newProgram->port = port;

        pthread_t new_program_thread;
        pthread_create(&new_program_thread, NULL, create_new_program, (void*)newProgram);
        pthread_detach(new_program_thread);
    }

    //--Funcion que se ejecuta cuando se pierde la conexion con un cliente
    void lost(int fd, char * ip, int port){
        pthread_mutex_lock(&mutex_logger);
        log_trace(logger, "El programa en IP:%s, PORT:%d ha muerto", ip, port);
        pthread_mutex_unlock(&mutex_logger);
        //TODO:remove program from programs list
    }

    //--funcion que se ejecuta cuando se recibe un nuevo mensaje de un cliente ya conectado
    void incoming(int fd, char * ip, int port, MessageHeader * headerStruct){
        printf("Esperando mensaje...\n");

        t_list *cosas = receive_package(fd, headerStruct);

        t_new_comm* newComm = malloc(sizeof(t_new_comm*));
        newComm->fd = fd;
        newComm->ip = malloc(sizeof(char*));
        newComm->ip = ip;
        newComm->port = port;
        newComm->received = malloc(sizeof(t_list*));
        newComm->received = cosas;


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
            case SUSE_CREATE:
            {
                pthread_t suse_create_thread;
                pthread_create(&suse_create_thread, NULL, suse_create, (void*)newComm);
                pthread_detach(suse_create_thread);
                break;
            }
            case SUSE_SCHEDULE_NEXT:
            {
                pthread_t suse_schedule_next_thread;
                pthread_create(&suse_schedule_next_thread, NULL, suse_schedule_next, (void*)newComm);
                pthread_detach(suse_schedule_next_thread);
                break;
            }
            case SUSE_WAIT:
            {
                pthread_t suse_wait_thread;
                pthread_create(&suse_wait_thread, NULL, suse_wait, (void*)newComm);
                pthread_detach(suse_wait_thread);
                break;
            }
            case SUSE_SIGNAL:
            {
                pthread_t suse_signal_thread;
                pthread_create(&suse_signal_thread, NULL, suse_signal, (void*)newComm);
                pthread_detach(suse_signal_thread);
                break;
            }
            case SUSE_JOIN:
            {
                pthread_t suse_join_thread;
                pthread_create(&suse_join_thread, NULL, suse_join, (void*)newComm);
                pthread_detach(suse_join_thread);
                break;
            }
            case SUSE_RETURN:
            {
                pthread_t suse_return_thread;
                pthread_create(&suse_return_thread, NULL, suse_return, (void*)newComm);
                pthread_detach(suse_return_thread);
                break;
            }
            default:
            {
                printf("Operacion desconocida. No quieras meter la pata\n");
                break;
            }
        }

    }
    pthread_mutex_lock(&mutex_logger);
    log_trace(logger, "Hilo de servidor iniciado...");
    pthread_mutex_unlock(&mutex_logger);
    start_server(socket, &new, &lost, &incoming);
}

void* create_new_program(void* programaNuevoData){
    t_newProgramData *newProgramData = (t_newProgramData*)programaNuevoData;
    char* ip = newProgramData->ip;
    int port = newProgramData->port;
    char* PID = generate_pid(ip, port);
    t_programa* nuevo_programa = (t_programa*)malloc(sizeof(t_programa));
    nuevo_programa->identificador = PID;
    nuevo_programa->ready = list_create();
    nuevo_programa->exec = malloc(sizeof(t_thread*));

    pthread_mutex_lock(&mutex_programs);
    dictionary_put(programs, PID, (void*)nuevo_programa);
    pthread_mutex_unlock(&mutex_programs);

    pthread_mutex_lock(&mutex_logger);
    log_trace(logger, "Nuevo programa agregado, PID:%s", PID);
    log_trace(logger, "Nivel de multiprogramacion:%d", dictionary_size(programs));
    pthread_mutex_unlock(&mutex_logger);
}

void* metrics_function(void* arg){
    pthread_mutex_lock(&mutex_logger);
    log_trace(logger, "Hilo de metricas iniciado...");
    pthread_mutex_unlock(&mutex_logger);
    while(1){
        sleep(config->metrics_timer);
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
    string_append(&metric_to_log, separator);

    pthread_mutex_lock(&mutex_logger);
    log_info(logger, metric_to_log);
    pthread_mutex_unlock(&mutex_logger);

    free(metric_to_log);
}

char* generate_thread_metrics(){}

char* generate_program_metrics(){}

char* generate_system_metrics(){}

void* suse_create(void* newComm){
    t_new_comm* newComm1 = (t_new_comm*)newComm;
    int fd = newComm1->fd;
    char* ip = newComm1->ip;
    int port = newComm1->port;
    t_list* received = newComm1->received;
}

void* suse_schedule_next(void* newComm){
    t_new_comm* newComm1 = (t_new_comm*)newComm;
    int fd = newComm1->fd;
    char* ip = newComm1->ip;
    int port = newComm1->port;
    t_list* received = newComm1->received;
}

void* suse_wait(void* newComm){
    t_new_comm* newComm1 = (t_new_comm*)newComm;
    int fd = newComm1->fd;
    char* ip = newComm1->ip;
    int port = newComm1->port;
    t_list* received = newComm1->received;
}

void* suse_signal(void* newComm){
    t_new_comm* newComm1 = (t_new_comm*)newComm;
    int fd = newComm1->fd;
    char* ip = newComm1->ip;
    int port = newComm1->port;
    t_list* received = newComm1->received;
}

void* suse_join(void* newComm){
    t_new_comm* newComm1 = (t_new_comm*)newComm;
    int fd = newComm1->fd;
    char* ip = newComm1->ip;
    int port = newComm1->port;
    t_list* received = newComm1->received;
}

void* suse_return(void* newComm){
    t_new_comm* newComm1 = (t_new_comm*)newComm;
    int fd = newComm1->fd;
    char* ip = newComm1->ip;
    int port = newComm1->port;
    t_list* received = newComm1->received;
}

char* generate_pid(char* ip, int port){
    char* new_pid = string_new();
    char* separator = "::";
    char* string_port = string_itoa(port);
    string_append(&new_pid, ip);
    string_append(&new_pid, separator);
    string_append(&new_pid, string_port);
    return new_pid;
}