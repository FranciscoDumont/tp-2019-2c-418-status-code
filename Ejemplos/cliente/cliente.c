//
// Created by utnso on 30/09/19.
//

#include "cliente.h"
#include <altaLibreria/connections.h>
#include <altaLibreria/structures.h>

int main(){
    //Creo un socket
    int socket_servidor = create_socket();
    int port = 4444;
    char* ip_server = "127.0.0.1";
    char* mensaje;

    //Pongo en escucha al socket en ese puerto
    if(-1 == bind_socket(socket_servidor, port)){
        printf("Error bind ::NOT FOUND\n");
    }else {
        printf("EL bind anda bien ::E\n");
    }

    //conecto el socket
    if(-1 == connect_socket(socket_servidor, ip_server, port)){
        printf("Error connect ::NOT FOUND\n");
    }else {
        printf("EL connect anda bien ::E \n");
    }

    //Ingreso el mensaje a enviar
    scanf("%s",mensaje);

    //Envio el mensaje con send_data
    int resultado = send_data(socket_servidor, ABC, sizeof(mensaje), mensaje);
    if(-1 == resultado){
        printf("Error envio ::NOT FOUND\n");
    }else {
        printf("Byts eviados: %d ::E\n", resultado);
    }

    //Habria que probar resivir algo del servidor
    MessageHeader* buffer_header;
    if(-1 == receive_header(socket_servidor,buffer_header)){
        printf("Error al recibir header ::NOT FOUND\n");
    }else {
        printf("Header recibido:\n type: %d\n size : %d ::E\n", buffer_header->type,buffer_header->data_size);
    }

    char* buffer_data;
    if(-1 == receive_data(socket_servidor,(void*) buffer_data,buffer_header->data_size)){
        printf("Error al recibir datos ::NOT FOUND\n");
    }else{
        printf("dato recibido:%s",buffer_data);
    }

    //Libero el socket
    close_socket(socket_servidor);

    return 0;
}