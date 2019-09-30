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

    //Recibo datos del servidor
    //Primero recibo el encabezado y el tamanio ya que son datos de tamaño fijo
    MessageHeader* buffer_header;
    if(-1 == receive_header(socket_servidor,buffer_header)){
        printf("Error al recibir header ::NOT FOUND\n");
    }else {
        printf("Header recibido:\n type: %d\n size : %d ::E\n", buffer_header->type,buffer_header->data_size);
    }

    //Despues recibo los datos usando la informacion del header
    char* buffer_data;
    if(-1 == receive_data(socket_servidor,(void*) buffer_data,buffer_header->data_size)){
        printf("Error al recibir datos ::NOT FOUND\n");
    }else{
        printf("dato recibido:%s",buffer_data);
    }

    //Libero el socket
    close_socket(socket_servidor);

    free(buffer_data);
    free(mensaje);

    return 0;
}