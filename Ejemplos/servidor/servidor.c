//
// Created by utnso on 30/09/19.
//

#include "servidor.h"

int main(){
    printf("Modulo que hace cosas locas\n");
    //pthread_t hiloLoco;
    //serverStart(&hiloLoco);
    serverFunction();
    return 0;
}

void serverFunction(){
    int socket;
    if((socket = create_socket()) == -1) {
        printf("Error al crear el socket");
        return;
    }
    if((bind_socket(socket, 4445)) == -1) {
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
                    t_paquete *package = create_package(ABC);
                    add_to_package(package, (void*) mensaje, strlen(mensaje) + 1);
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