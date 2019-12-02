//
// Created by utnso on 28/10/19.
//
#include "sac-server.h"

sac_server_config* read_config(){
    sac_server_config* confi = malloc(sizeof(sac_server_config));
    t_config* tConfig =config_create(CONFIGURACION);

    confi->ip = config_get_string_value(tConfig, "IP");
    confi->listen_port = atoi(config_get_string_value(tConfig, "LISTEN_PORT"));

    config_destroy(tConfig);
    return confi;
}

void serverFunction(){
    sac_server_config* config;
    int socket;
    t_log* logger = log_create("../sac_server.log", "SAC_SERVER", false, LOG_LEVEL_TRACE);;

    config = read_config();

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
                char* path = list_get(cosas,0);// Toma el unico elemento que se le manda
                int descriptor_archiv = sac_open(path);
                t_paquete *package = create_package(OPEN);
                add_to_package(package, (void*) &descriptor_archiv, sizeof(descriptor_archiv));
                send_package(package, fd);

                break;
            }
//            case CLOSE:
//            {
//                int* fd = list_get(cosas,0);// Toma el unico elemento que se le manda
//                int resultado = sac_close(*fd);
//                t_paquete *package = create_package(CLOSE);
//                add_to_package(package, (void*) &resultado, sizeof(resultado));
//                send_package(package, *fd);
//
//                break;
//            }

            case READ:
            {
                ;
                char *mensaje = (char*)list_get(cosas, 0);
                printf("Se quiso leer esta ruta:%s\n", mensaje);

                break;
            }
            case WRITE:
            {
                ;
                char *mensaje = (char*)list_get(cosas, 0);
                printf("Se quiso escribir en esta ruta:%s\n", mensaje);

                break;
            }
            case GETATTR:
            {
                ;
                char *mensaje = (char*)list_get(cosas, 0);
                printf("Se pidio informacion sobre esta ruta:%s\n", mensaje);
                break;
            }
            case MKNOD:
            {
                ;
                char *mensaje = (char*)list_get(cosas, 0);
                printf("Se creo un nuevo archivo con direccion:%s\n", mensaje);

                break;
            }
            case MKDIR:
            {
                ;
                char *mensaje = (char*)list_get(cosas, 0);
                printf("Se creo un nuevo directorio con direccion:%s\n", mensaje);

                break;
            }

            case READDIR:
            {
                ;
                char *mensaje = (char*)list_get(cosas, 0);
                printf("Se quiere leer el directorio:%s\n", mensaje);

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

