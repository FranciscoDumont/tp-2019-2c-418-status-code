//
// Created by utnso on 30/09/19.
//

#include "cliente.h"

int main(){
    //Creo un socket
    int socket_servidor = create_socket();
    int port = 5003;
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

    t_paquete *package = create_package(ABC);
    add_to_package(package, (void*) mensaje, strlen(mensaje) + 1);

    if(send_package(package, socket_servidor) == -1){
        printf("Error en el envio...\n");
    } else {
        printf("Mensaje enviado\n");
    }

    //Recibo datos del servidor
    //Primero recibo el encabezado y el tamanio ya que son datos de tamaño fijo
    MessageHeader* buffer_header = malloc(sizeof(MessageHeader));
    if(-1 == receive_header(socket_servidor, buffer_header)){
        printf("Error al recibir header ::NOT FOUND\n");
    }else {
        printf("Header recibido:\n type: %d\n size : %d ::E\n", buffer_header->type,buffer_header->data_size);
    }

    t_list *cosas = receive_package(socket_servidor, buffer_header);

    switch (buffer_header->type){
        case ABC:
        {
            ;
            char *mensaje = (char*)list_get(cosas, 0);
            printf("Mensaje recibido:%s\n", mensaje);
            break;
        }
        default:
        {
            printf("Operacion desconocida. No quieras meter la pata");
            break;
        }
    }

    //Libero el socket
    close_socket(socket_servidor);
    free(package);
    free(buffer_header);

    return 0;
}