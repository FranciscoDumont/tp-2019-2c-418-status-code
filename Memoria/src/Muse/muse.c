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

        switch (headerStruct->type){
            case MUSE_INIT:
            ;
            {
                // Guardo el entero que recibo en una variable
                int id = *((int*)list_get(cosas, 0));
                
                // Ejecuto muse_init
                int resultado =  muse_init(id, ip, port);

                // Le respondo a libMuse
                t_paquete *package = create_package(MUSE_INIT);
                void* respuesta = malloc(sizeof(int));
                *((int*)respuesta) = resultado;
                add_to_package(package, respuesta, sizeof(int));
                send_package(package, fd);
                break;
            }

            case MUSE_CLOSE:
            ;
            {
                muse_close();
                break;
            }
            
            case MUSE_ALLOC:
            ;
            //todo: hacer como aca y por cada case ejecutar la funcion correspondiente
            //todo: lo que contiene cosas varía segun el orden de los parametros de cada funcion
            {
                // Guardo el valor que recibo en una variable
                uint32_t tam = *((uint32_t*)list_get(cosas, 0));
                muse_alloc(tam);
                break;
            }
            
            case MUSE_FREE:
            ;
            {
                uint32_t dir = *((uint32_t*)list_get(cosas, 0));
                muse_free(dir);
                break;
            }
            
            case MUSE_GET:
            ;
            {
                //muse_get(*dst,src,n);
                break;
            }
            
            case MUSE_CPY:
            ;
            {
                //muse_cpy(dst,*src,n);
                break;
            }
            
            case MUSE_MAP:
            ;
            {
                //muse_map(*path,length,flags);
                break;
            }
            
            case MUSE_SYNC:
            ;
            {
                //muse_sync(addr,len);
                break;
            }
            
            case MUSE_UNMAP:
            ;
            {
                //muse_unmap(dir);
                break;
            }

            default:
            {
                log_warning(logger, "Operacion desconocida. No quieras meter la pata\n");
                break;
            }
        }
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


int muse_init(int id, char *ip, int puerto){
    log_info(logger, "Empieza el muse_init");

    // Loggeo el valor del id que recibi
    log_info(logger, "El id que recibo de libMuse es: %d", id);
    
    return 1;
}


void muse_close(){
}


uint32_t muse_alloc(uint32_t tam){
    // Loggeo el valor del id que recibi
    log_info(logger, "El id que recibo de libMuse es: %d", tam);
}

void muse_free(uint32_t dir){
}


int muse_get(void *dst, uint32_t src, size_t n){
}


int muse_cpy(uint32_t dst, void *src, int n){
}


uint32_t muse_map(char *path, size_t length, int flags){
}


int muse_sync(uint32_t addr, size_t len){
}


int muse_unmap(uint32_t dir){
}
