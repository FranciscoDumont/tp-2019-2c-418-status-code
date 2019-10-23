#include <stdio.h>

#define PUERTO_BIND 4445
//Hay q agregarle los logs
//Probarlo
int main() {
    //Crear el socket
    int socket;
    if((socket = create_socket()) == -1) {
        printf("Error al crear el socket");
        return;
    }
    if((bind_socket(socket, PUERTO_BIND)) == -1) {
        printf("Error al bindear el socket");
        return;
    }
    //Crear el servidor
    start_server( Socket , void (*new_connection) ,void (*lost_connection), void (*incoming_message) )
}


void new_connection (int socket_cliente, char * ip_cliente, int port_de_coneccion){
    printf("Cliente conectado, IP:%s, PORT:%d\n", ip, port);
}

void los_connection (int fd, char * ip, int port){
    printf("Cliente Perdido, IP:%s, PORT:%d\n", ip, port);
}

void incoming_message (int fd, char * ip, int port, MessageHeader * headerStruct)){
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