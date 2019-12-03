//
// Created by utnso on 28/10/19.
//
#include "sac-server.h"

sac_server_config* read_config(){
    sac_server_config* confi = malloc(sizeof(sac_server_config));
    t_config* tConfig =config_create("../sac_server.config");

    confi->ip = config_get_string_value(tConfig, "IP");
    confi->listen_port = atoi(config_get_string_value(tConfig, "LISTEN_PORT"));

    config_destroy(tConfig);
    return confi;
}

void inicializarEjemplo(){
    char* tuVieja = malloc(50);
    memcpy(tuVieja,"/Carpeta1",strlen("/Carpeta1")+1);
    sac_mkdir(tuVieja);
    memcpy(tuVieja,"/Carpeta2",strlen("/Carpeta2")+1);
    sac_mkdir(tuVieja);
    memcpy(tuVieja,"/Carpeta1/archivo1",strlen("/Carpeta1/archivo1")+1);
    sac_mkdir(tuVieja);
    memcpy(tuVieja,"/Carpeta1/Carpeta3",strlen("/Carpeta1/Carpeta3")+1);
    sac_mkdir(tuVieja);
    memcpy(tuVieja,"/Carpeta1/algo",strlen("/Carpeta1/algo")+1);
    sac_mkdir(tuVieja);
    memcpy(tuVieja,"/Carpeta2/archivo1",strlen("/Carpeta2/archivo1")+1);
    sac_mkdir(tuVieja);
    free(tuVieja);
}


void serverFunction(){
    sac_server_config* config;
    int socket;
    t_log* logger = log_create("sac_server.log", "SAC_SERVER", false, LOG_LEVEL_TRACE);;

    config = read_config();
    incializarTablaArchivos();
    inicializarEjemplo();

    if((socket = create_socket()) == -1) {
        log_error(logger, "Error al crear el socket");
        return;
    }

    if((bind_socket(socket, 5005)) == -1) {
        log_error(logger, "Error al bindear el socket");
        return;
    }
    void new(int fd, char * ip, int port){
        log_info(logger, "Cliente conectado, IP:%s, PORT:%d\n", ip, port);
    }

    void lost(int fd, char * ip, int port){
        log_info(logger, "Cliente conectado, IP:%s, PORT:%d\n", ip, port);
    }

    void incoming(int fd, char * ip, int port, MessageHeader * headerStruct){
        t_list *cosas = receive_package(fd, headerStruct);

        switch (headerStruct->type){
            case OPEN:
            {
                //Toma lo que se mando
                char* path = list_get(cosas,0);// Toma el unico elemento que se le manda
                int descriptor_archivo = sac_open(path);

                //Envia la respuesta
                t_paquete *package = create_package(OPEN);
                add_to_package(package,(void*) &descriptor_archivo, sizeof(int));
                if(send_package(package, fd) == -1){
                    log_debug(logger, "Se ejecuto un OPEN con el path: %s",path);
                } else {
                    log_error(logger, "Error al enviar la respuesta del open() al socket: %d",fd);
                }
                free_package(package);
                free(path);

                break;
            }
            case CLOSE:
            {
                int* descriptor_archivo = list_get(cosas,0);// Toma el unico elemento que se le manda
                int resultado = sac_close(*descriptor_archivo);

                t_paquete *package = create_package(CLOSE);
                add_to_package(package, (void*) &resultado, sizeof(int));

                if(send_package(package, fd) == -1){
                    log_debug(logger, "Se ejecuto un CLOSE en fileDescriptor: %d",descriptor_archivo);
                } else {
                    log_error(logger, "Error al enviar la respuesta del close() al socket: %d",fd);
                }
                free_package(package);
                free(descriptor_archivo);

                break;
            }

//            case READ:
//            {
//                ;
//                char *mensaje = (char*)list_get(cosas, 0);
//                printf("Se quiso leer esta ruta:%s\n", mensaje);
//
//                break;
//            }
//            case WRITE:
//            {
//                ;
//                char *mensaje = (char*)list_get(cosas, 0);
//                printf("Se quiso escribir en esta ruta:%s\n", mensaje);
//
//                break;
//            }
            case GETATTR:
            {   struct stat stbuf;
                sac_getattr(const char *path, &stbuf);

                t_paquete *package = create_package(GETATTR);
                add_to_package(package, (void*) &stbuf, sizeof(struct stat));

                if(send_package(package, fd) == -1){
                    log_debug(logger, "Se ejecuto el comando GETATTR con el path: %s",path);
                } else {
                    log_error(logger, "Error al enviar la respuesta de getattr() al socket: %d",fd);
                }

                break;
            }
            case MKNOD:
            {
                char* path = list_get(cosas,0);
                int resultado = sac_mknod(path);

                t_paquete *package = create_package(MKNOD);
                add_to_package(package, (void*) &resultado, sizeof(int));

                if(send_package(package, fd) == -1){
                    log_debug(logger, "Se ejecuto el comando MKNOD con el path: %s",path);
                } else {
                    log_error(logger, "Error al enviar la respuesta de mknod() al socket: %d",fd);
                }

                free_package(package);
                free(path);

                break;
            }
            case MKDIR:
            {
                char* path = list_get(cosas,0);
                int resultado = sac_mkdir(path);

                t_paquete *package = create_package(MKDIR);
                add_to_package(package, (void*) &resultado, sizeof(int));

                if(send_package(package, fd) == -1){
                    log_debug(logger, "Se ejecuto el comando MKDIR con el path: %s",path);
                } else {
                    log_error(logger, "Error al enviar la respuesta de mkdir() al socket: %d",fd);
                }

                free_package(package);
                free(path);
                break;
            }

            case READDIR:
            {
                char* path = list_get(cosas,0);
                t_list* listado_directorios = list_create();
                int resultado = sac_mkdir(path, listado_directorios);
                t_paquete *package = create_package(READDIR);
                if(resultado >= 0){
                    add_to_package(package, (void*) &resultado, sizeof(int));
                    for(int i = 0; i >list_size(listado_directorios); i++){
                        Dir* directorio = list_get(listado_directorios, i);
                        add_to_package(package, directorio, sizeof(dir));
                    }
                }else{
                    add_to_package(package, (void*) &resultado, sizeof(int));
                }
                if(send_package(package, fd) == -1){
                    log_debug(logger, "Se ejecuto el comando READDIR con el path: %s",path);
                } else {
                    log_error(logger, "Error al enviar la respuesta de readdir() al socket: %d",fd);
                }

                free_package(package);
                free(path);
                break;
            }

            case RMDIR:
            {
                char* path = list_get(cosas,0);
                int resultado = sac_rmdir(path);

                t_paquete *package = create_package(RMDIR);
                add_to_package(package, (void*) &resultado, sizeof(int));

                if(send_package(package, fd) == -1){
                    log_debug(logger, "Se ejecuto la instruccion RMDIR con el siguiente: %s",path);
                } else {
                    log_error(logger, "Error al enviar la respuesta de rmdir() al socket: %d",fd);
                }

                free_package(package);
                free(path);

                break;
            }

            case UNLINK:
            {
                char* path = list_get(cosas,0);
                int resultado = sac_rmnod(path);

                t_paquete *package = create_package(UNLINK);
                add_to_package(package, (void*) &resultado, sizeof(int));

                if(send_package(package, fd) == -1){
                    log_debug(logger, "Se ejecuto la instruccion UNLINK(rmnode) con el siguiente: %s",path);
                } else {
                    log_error(logger, "Error al enviar la respuesta de unlink() al socket: %d",fd);
                }

                free_package(package);
                free(path);
                break;
            }

            default:
            {
                printf("Operacion desconocida. No quieras meter la pata");
                break;
            }
        }

    }
    start_server(socket, &new, &lost, &incoming);
}


int main(){
    printf("Modulo que hace cosas locas\n");
    //pthread_t hiloLoco;
    //serverStart(&hiloLoco);
    serverFunction();
    return 0;
}

