#include "muse.h"


int main() {
    logger = log_create("muse.log", "MUSE", 1, LOG_LEVEL_TRACE);
    read_memory_config();

    process_table = list_create();

    main_memory = malloc(config.memory_size);


    pthread_t server_thread;
    pthread_create(&server_thread, NULL, server_function, NULL);
    pthread_join(server_thread, NULL);


    return 0;
}


void* server_function(void * arg){

    int port = config.listen_port;
    int socket;

    if((socket = create_socket()) == -1) {
        log_error(logger, "Error al crear el socket");
        return (void *) -1;
    }
    if((bind_socket(socket, port)) == -1) {
        log_error(logger, "Error al bindear el socket");
        return (void *) -2;
    }

    //--Funcion que se ejecuta cuando se conecta un nuevo programa
    void new(int fd, char * ip, int port){
        log_info(logger, "Nueva conexión");
    }

    //--Funcion que se ejecuta cuando se pierde la conexion con un cliente
    void lost(int fd, char * ip, int port){
        log_info(logger, "Se perdió una conexión");
    }

    //--funcion que se ejecuta cuando se recibe un nuevo mensaje de un cliente ya conectado
    void incoming(int fd, char* ip, int port, MessageHeader * headerStruct){

        t_list* cosas = receive_package(fd, headerStruct);

        t_new_comm* newComm = malloc(sizeof(t_new_comm));
        newComm->fd = fd;
        newComm->ip = malloc(sizeof(char));
        newComm->ip = ip;
        newComm->port = port;
        newComm->received = malloc(sizeof(t_list));
        newComm->received = cosas;

        switch (headerStruct->type){
            case MUSE_INIT:
            {
                pthread_t muse_init_thread;
                pthread_create(&muse_init_thread, NULL, muse_init, (void*)newComm);
                pthread_join(muse_init_thread, NULL);
                break;
            }
            default:
            {
                log_warning(logger, "Operacion desconocida. No quieras meter la pata\n");
                break;
            }
        }

        free(newComm);

    }
    log_info(logger, "Hilo de servidor iniciado...");
    start_server(socket, &new, &lost, &incoming);
}


void read_memory_config(){
    config_file = config_create("config");

    if (!config_file){
        log_error(logger, "No se encontró el archivo de configuración");
        return;
    }

    config.listen_port = config_get_int_value(config_file, "LISTEN_PORT");
    config.memory_size = config_get_int_value(config_file, "MEMORY_SIZE");
    config.page_size = config_get_int_value(config_file, "PAGE_SIZE");
    config.swap_size = config_get_int_value(config_file, "SWAP_SIZE");

    log_info(logger, \
        "Configuración levantada\n\tLISTEN_PORT: %d\n\tMEMORY_SIZE: %d\n\tPAGE_SIZE: %d\n\tSWAP_SIZE: %d.",\
        config.listen_port, \
        config.memory_size, \
        config.page_size, \
        config.swap_size);
}


void* muse_init(void* newComm){
    log_error(logger, "Empieza el muse_init");
    t_new_comm* newComm1 = (t_new_comm*)newComm;
    int fd = newComm1->fd;
    char* ip = newComm1->ip;
    int port = newComm1->port;
    t_list* received = newComm1->received;

    // Guardo el entero que recibo en una variable
    int id = *((int*)list_get(received, 0));

    // Loggeo el valor del id que recibi
    log_info(logger, "El id que recibo de libMuse es: %d", id);

    // Le respondo a libMuse que esta todo bien
    t_paquete *package = create_package(MUSE_INIT);
    void* confirmation = malloc(sizeof(int));
    *((int*)confirmation) = 1;
    add_to_package(package, confirmation, sizeof(int));
    send_package(package, fd);
}


void muse_close(){
    log_error(logger, "Empieza el muse_close");
    t_new_comm* newComm1 = (t_new_comm*)newComm;
    int fd = newComm1->fd;
    char* ip = newComm1->ip;
    int port = newComm1->port;
    t_list* received = newComm1->received;

    // Guardo el entero que recibo en una variable
    int id = *((int*)list_get(received, 0));

    // Loggeo el valor del id que recibi
    log_info(logger, "El id que recibo de libMuse es: %d", id);

    // Le respondo a libMuse que esta todo bien
    t_paquete *package = create_package(MUSE_INIT);
    void* confirmation = malloc(sizeof(int));
    *((int*)confirmation) = 1;
    add_to_package(package, confirmation, sizeof(int));
    send_package(package, fd);
}


uint32_t muse_alloc(uint32_t tam){
    log_error(logger, "Empieza el muse_alloc");
    t_new_comm* newComm1 = (t_new_comm*)newComm;
    int fd = newComm1->fd;
    char* ip = newComm1->ip;
    int port = newComm1->port;
    t_list* received = newComm1->received;

    // Guardo el entero que recibo en una variable
    int id = *((int*)list_get(received, 0));

    // Loggeo el valor del id que recibi
    log_info(logger, "El id que recibo de libMuse es: %d", id);

    // Le respondo a libMuse que esta todo bien
    t_paquete *package = create_package(MUSE_INIT);
    void* confirmation = malloc(sizeof(int));
    *((int*)confirmation) = 1;
    add_to_package(package, confirmation, sizeof(int));
    send_package(package, fd);
}

void muse_free(uint32_t dir){
    log_error(logger, "Empieza el muse_free");
    t_new_comm* newComm1 = (t_new_comm*)newComm;
    int fd = newComm1->fd;
    char* ip = newComm1->ip;
    int port = newComm1->port;
    t_list* received = newComm1->received;

    // Guardo el entero que recibo en una variable
    int id = *((int*)list_get(received, 0));

    // Loggeo el valor del id que recibi
    log_info(logger, "El id que recibo de libMuse es: %d", id);

    // Le respondo a libMuse que esta todo bien
    t_paquete *package = create_package(MUSE_INIT);
    void* confirmation = malloc(sizeof(int));
    *((int*)confirmation) = 1;
    add_to_package(package, confirmation, sizeof(int));
    send_package(package, fd);
}


int muse_get(void *dst, uint32_t src, size_t n){
    log_error(logger, "Empieza el muse_get");
    t_new_comm* newComm1 = (t_new_comm*)newComm;
    int fd = newComm1->fd;
    char* ip = newComm1->ip;
    int port = newComm1->port;
    t_list* received = newComm1->received;

    // Guardo el entero que recibo en una variable
    int id = *((int*)list_get(received, 0));

    // Loggeo el valor del id que recibi
    log_info(logger, "El id que recibo de libMuse es: %d", id);

    // Le respondo a libMuse que esta todo bien
    t_paquete *package = create_package(MUSE_INIT);
    void* confirmation = malloc(sizeof(int));
    *((int*)confirmation) = 1;
    add_to_package(package, confirmation, sizeof(int));
    send_package(package, fd);
}


int muse_cpy(uint32_t dst, void *src, int n){
    log_error(logger, "Empieza el muse_cpy");
    t_new_comm* newComm1 = (t_new_comm*)newComm;
    int fd = newComm1->fd;
    char* ip = newComm1->ip;
    int port = newComm1->port;
    t_list* received = newComm1->received;

    // Guardo el entero que recibo en una variable
    int id = *((int*)list_get(received, 0));

    // Loggeo el valor del id que recibi
    log_info(logger, "El id que recibo de libMuse es: %d", id);

    // Le respondo a libMuse que esta todo bien
    t_paquete *package = create_package(MUSE_INIT);
    void* confirmation = malloc(sizeof(int));
    *((int*)confirmation) = 1;
    add_to_package(package, confirmation, sizeof(int));
    send_package(package, fd);
}


uint32_t muse_map(char *path, size_t length, int flags){
    log_error(logger, "Empieza el muse_map");
    t_new_comm* newComm1 = (t_new_comm*)newComm;
    int fd = newComm1->fd;
    char* ip = newComm1->ip;
    int port = newComm1->port;
    t_list* received = newComm1->received;

    // Guardo el entero que recibo en una variable
    int id = *((int*)list_get(received, 0));

    // Loggeo el valor del id que recibi
    log_info(logger, "El id que recibo de libMuse es: %d", id);

    // Le respondo a libMuse que esta todo bien
    t_paquete *package = create_package(MUSE_INIT);
    void* confirmation = malloc(sizeof(int));
    *((int*)confirmation) = 1;
    add_to_package(package, confirmation, sizeof(int));
    send_package(package, fd);
}


int muse_sync(uint32_t addr, size_t len){
    log_error(logger, "Empieza el muse_sync");
    t_new_comm* newComm1 = (t_new_comm*)newComm;
    int fd = newComm1->fd;
    char* ip = newComm1->ip;
    int port = newComm1->port;
    t_list* received = newComm1->received;

    // Guardo el entero que recibo en una variable
    int id = *((int*)list_get(received, 0));

    // Loggeo el valor del id que recibi
    log_info(logger, "El id que recibo de libMuse es: %d", id);

    // Le respondo a libMuse que esta todo bien
    t_paquete *package = create_package(MUSE_INIT);
    void* confirmation = malloc(sizeof(int));
    *((int*)confirmation) = 1;
    add_to_package(package, confirmation, sizeof(int));
    send_package(package, fd);
}


int muse_unmap(uint32_t dir){
    log_error(logger, "Empieza el muse_unmap");
    t_new_comm* newComm1 = (t_new_comm*)newComm;
    int fd = newComm1->fd;
    char* ip = newComm1->ip;
    int port = newComm1->port;
    t_list* received = newComm1->received;

    // Guardo el entero que recibo en una variable
    int id = *((int*)list_get(received, 0));

    // Loggeo el valor del id que recibi
    log_info(logger, "El id que recibo de libMuse es: %d", id);

    // Le respondo a libMuse que esta todo bien
    t_paquete *package = create_package(MUSE_INIT);
    void* confirmation = malloc(sizeof(int));
    *((int*)confirmation) = 1;
    add_to_package(package, confirmation, sizeof(int));
    send_package(package, fd);
}
