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
                send_package(package, fd);
                break;
            }
            case CLOSE:
            {
                int* fd = list_get(cosas,0);// Toma el unico elemento que se le manda
                int resultado = sac_close(*fd);
                t_paquete *package = create_package(CLOSE);
                add_to_package(package, (void*) &resultado, sizeof(resultado)+1);
                send_package(package, *fd);

                break;
            }

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

