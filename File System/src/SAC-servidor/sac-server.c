//
// Created by utnso on 28/10/19.
//
#include "sac-server.h"
#include "funciones.h"

void serverFunction(){

    //sac_server_config* config;
    int socket;

    //config = read_config();

    if((socket = create_socket()) == -1) {
        printf("Error al crear el socket");
        return;
    }
    if((bind_socket(socket, 5005)) == -1) {
        printf("Error al bindear el socket");
        return;
    }
    void new(int fd, char * ip, int port){
        printf("Cliente conectado, IP:%s, PORT:%d\n", ip, port);
    }

    void lost(int fd, char * ip, int port){}
    void incoming(int fd, char * ip, int port, MessageHeader * headerStruct){
        t_list *cosas = receive_package(fd, headerStruct);

        switch (headerStruct->type){
            case OPEN:
            {
                ;
                char *mensaje = (char*)list_get(cosas, 0);
                printf("Se pidio abrir esta ruta :%s\n", mensaje);

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

