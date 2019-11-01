//
// Created by utnso on 28/10/19.
//
#include "sac-server.h"


int main(){
    printf("Modulo que hace cosas locas\n");
    //pthread_t hiloLoco;
    //serverStart(&hiloLoco);
    serverFunction();
    return 0;
}

void serverFunction(){

    sac_server_config* config;
    int socket;

    config = read_config();

    if((socket = create_socket()) == -1) {
        printf("Error al crear el socket");
        return;
    }
    if((bind_socket(socket, config->listen_port)) == -1) {
        printf("Error al bindear el socket");
        return;
    }
    void new(int fd, char * ip, int port){
        printf("Cliente conectado, IP:%s, PORT:%d\n", ip, port);
    }

    void lost(int fd, char * ip, int port){}
    void incoming(int fd, char * ip, int port, MessageHeader * headerStruct){
        printf("Esperando mensaje...\n");

        t_list *cosas = receive_package(fd, headerStruct);

        switch (headerStruct->type){
            case ABC:
            {
                ;
                char *mensaje = (char*)list_get(cosas, 0);
                printf("Mensaje recibido:%s\n", mensaje);
                t_paquete *package = crear_paquete(ABC);
                agregar_a_paquete(package, (void*) mensaje, strlen(mensaje) + 1);
                if(send_package(package, fd) == -1){
                    printf("Error en el envio...\n");
                } else {
                    printf("Mensaje enviado\n");
                }
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

sac_server_config* read_config(){
    sac_server_config* config;
    t_log* logger;

    config = malloc(sizeof(sac_server_config));

    logger = log_create("../sac_server.log", "sac-server", 1, LOG_LEVEL_TRACE);


    t_config* config_file = config_create("../sac_server.config");
    config->listen_port = config_get_int_value(config_file, "LISTEN_PORT");
    config->ip = config_get_string_value(config_file, "IP");

    log_trace(logger,
              "Config file read: LISTEN_PORT: %d, IP: %s",
              config->listen_port,
              config->ip
    );
    config_destroy(config_file);

    return config;
}
