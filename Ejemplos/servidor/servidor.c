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
        printf("Cliente conectado, IP:%s\n", ip);
    }

    void lost(int fd, char * ip, int port){}
    void incoming(int fd, char * ip, int port, MessageHeader * headerStruct){
        printf("Esperando mensaje...\n");
        printf("Tamaño de mensaje a recibir: %i\n", headerStruct->data_size);
        int size = headerStruct->data_size;
        MessageType header = headerStruct->type;
        char *message = malloc(size);
        if(receive_data(fd, (void *)message, size) == -1){
            return;
        }
        printf("El mensaje recibido es.. %s\n", message);

        printf("Ahora devuelvo el mismo mensaje al cliente..\n");
        if(send_data(fd, header, size, (void *) message) == -1){
            return;
        }
        free(message);
    }
    start_server(socket, &new, &lost, &incoming);
}