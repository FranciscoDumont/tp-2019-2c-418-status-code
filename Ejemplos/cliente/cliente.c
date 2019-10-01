//
// Created by utnso on 30/09/19.
//

#include "cliente.h"

int main(){
    //Creo un socket
    int socket_servidor = create_socket();
    int port = 4445;
    char* ip_server = "127.0.0.1";
    char* mensaje = malloc(20);

    //conecto el socket
    if(-1 == connect_socket(socket_servidor, ip_server, port)){
        printf("Error connect ::NOT FOUND\n");
    }else {
        printf("EL connect anda bien ::E \n");
    }

    printf("Escriba un mensaje a enviar: ");
    //Ingreso el mensaje a enviar
    scanf("%s",mensaje);

    //Envio el mensaje con send_data
    int resultado = send_data(socket_servidor, ABC, strlen(mensaje), mensaje);
    if(-1 == resultado){
        printf("Error envio ::NOT FOUND\n");
    }else {
        printf("Bytes enviados: %d ::E\n", resultado);
    }

    //Recibo datos del servidor
    //Primero recibo el encabezado y el tamanio ya que son datos de tamaño fijo
    MessageHeader* buffer_header = malloc(sizeof(MessageHeader));
    if(-1 == receive_header(socket_servidor, buffer_header)){
        printf("Error al recibir header ::NOT FOUND\n");
    }else {
        printf("Header recibido:\n type: %d\n size : %d ::E\n", buffer_header->type,buffer_header->data_size);
    }

    //Despues recibo los datos usando la informacion del header
    char* buffer_data = malloc(buffer_header->data_size);
    if(-1 == receive_data(socket_servidor,(void*) buffer_data,buffer_header->data_size)){
        printf("Error al recibir datos ::NOT FOUND\n");
    }else{
        printf("dato recibido:%s\n",buffer_data);
    }

    //Libero el socket
    close_socket(socket_servidor);

    free(buffer_data);
    free(mensaje);

    return 0;
}