//
// Created by utnso on 30/09/19.
//

#include "servidor.h"

int main(){
    printf("Modulo que hace cosas locas");
    printf("Creo un hilo que al que le voy a pasar una funcion que inicia un servidor y se queda esperando por clientes");
    //pthread_t hiloLoco;
    //serverStart(&hiloLoco);
    serverFunction();
    return 0;
}

void serverFunction(){
    int socket;
    if((socket = create_socket()) == -1) {
        return;
    }
    if((bind_socket(socket, 4444)) == -1) {
        return;
    }
    void new(int fd, char * ip, int port){
        printf("Cliente conectado...\n");
        printf("Esperando mensaje...\n");
        MessageHeader headerStruct;
        if(receive_header(socket, &headerStruct) == -1){
            return;
        }
        int size = headerStruct.data_size;
        MessageType header = headerStruct.type;
        char *message = malloc(sizeof(char)*size);
        if(receive_data(socket, message, size) == -1){
            return;
        }
        printf("El mensaje recibido es: %s\n", message);

        printf("Ahora devuelvo el mismo mensaje al cliente..\n");
        if(send_data(fd, header, size, (void *) message) == -1){
            return;
        }
        free(message);
    }

    void lost(int fd, char * ip, int port){}
    void incoming(int fd, char * ip, int port, MessageHeader * header){}
    start_server(socket, &new, &lost, &incoming);
}