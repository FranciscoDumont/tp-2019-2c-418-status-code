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
            ;
            {
                pthread_t muse_init_thread;
                pthread_create(&muse_init_thread, NULL, muse_init, (void*)newComm);
                pthread_join(muse_init_thread, NULL);
                break;
            }

            case MUSE_CLOSE:
            ;
            {
                pthread_t muse_close_thread;
                pthread_create(&muse_close_thread, NULL, muse_close, (void*)newComm);
                pthread_join(muse_close_thread, NULL);
                break;
            }
            
            case MUSE_ALLOC:
            ;
            {
                pthread_t muse_alloc_thread;
                pthread_create(&muse_alloc_thread, NULL, muse_alloc, (void*)newComm);
                pthread_join(muse_alloc_thread, NULL);
                break;
            }
            
            case MUSE_FREE:
            ;
            {
                pthread_t muse_free_thread;
                pthread_create(&muse_free_thread, NULL, muse_free, (void*)newComm);
                pthread_join(muse_free_thread, NULL);
                break;
            }
            
            case MUSE_GET:
            ;
            {
                pthread_t muse_get_thread;
                pthread_create(&muse_get_thread, NULL, muse_get, (void*)newComm);
                pthread_join(muse_get_thread, NULL);
                break;
            }
            
            case MUSE_CPY:
            ;
            {
                pthread_t muse_cpy_thread;
                pthread_create(&muse_cpy_thread, NULL, muse_cpy, (void*)newComm);
                pthread_join(muse_cpy_thread, NULL);
                break;
            }
            
            case MUSE_MAP:
            ;
            {
                pthread_t muse_map_thread;
                pthread_create(&muse_map_thread, NULL, muse_map, (void*)newComm);
                pthread_join(muse_map_thread, NULL);
                break;
            }
            
            case MUSE_SYNC:
            ;
            {
                pthread_t muse_sync_thread;
                pthread_create(&muse_sync_thread, NULL, muse_sync, (void*)newComm);
                pthread_join(muse_sync_thread, NULL);
                break;
            }
            
            case MUSE_UNMAP:
            ;
            {
                pthread_t muse_unmap_thread;
                pthread_create(&muse_unmap_thread, NULL, muse_unmap, (void*)newComm);
                pthread_join(muse_unmap_thread, NULL);
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


void muse_init(void* newComm){
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


void muse_close(void* newComm){
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


//uint32_t muse_alloc(uint32_t tam){
void muse_alloc(void* newComm){
    log_info(logger, "Empieza el muse_alloc");
    t_new_comm* newComm1 = (t_new_comm*)newComm;
    int fd = newComm1->fd;
    t_list* received = newComm1->received;

    // Guardo el entero que recibo en una variable
    uint32_t tam = *((uint32_t*)list_get(received, 0));

    // Loggeo el valor del id que recibi
    log_info(logger, "El id que recibo de libMuse es: %d", tam);

    // Le respondo a libMuse que esta todo bien
    t_paquete *package = create_package(MUSE_INIT);
    void* confirmation = malloc(sizeof(int));
    *((int*)confirmation) = 1;
    add_to_package(package, confirmation, sizeof(int));
    send_package(package, fd);
}

//void muse_free(uint32_t dir){
void muse_free(void* newComm){
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


//int muse_get(void *dst, uint32_t src, size_t n){
void muse_get(void* newComm){
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


//int muse_cpy(uint32_t dst, void *src, int n){
void muse_cpy(void* newComm){
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


//uint32_t muse_map(char *path, size_t length, int flags){
void muse_map(void* newComm){
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


//int muse_sync(uint32_t addr, size_t len){
void muse_sync(void* newComm){
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


//int muse_unmap(uint32_t dir){
void muse_unmap(void* newComm){
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
