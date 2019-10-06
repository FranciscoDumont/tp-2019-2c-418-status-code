#include "suse.h"

t_config *config = NULL;
//SUSEConfig

int main() {

    void* thread_server_error;
    pthread_t server_thread;

    //TODO:Decidir si trabajo con el t_config o con mi propia estructura
    config = leerConfig();

    pthread_create(&server_thread, NULL, server_function, NULL);

    pthread_join(server_thread, &thread_server_error);

    //TODO:Loguear el error del servidor
    //int server_error = (int) (intptr_t) thread_server_error;

    return 0;
}

t_config *leerConfig(){
    return config_create("../suse.config");
}

void* server_function(void * arg){
    int PORT = config_get_string_value(config, "LISTEN_PORT);
    int socket;
    if((socket = create_socket()) == -1) {
        printf("Error al crear el socket");
        return (void *) -1;
    }
    if((bind_socket(socket, PORT)) == -1) {
        printf("Error al bindear el socket");
        return (void *) -2;
    }
    void new(int fd, char * ip, int port){
        printf("Cliente conectado, IP:%s, PORT:%d\n", ip, port);
    }

    void lost(int fd, char * ip, int port){
        printf("El cliente conectado en IP:%s, PORT:%d, ha muerto...\n", ip, port);
    }
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
            case SUSE_INIT:
            {
                printf("Iniciar");
                break;
            }
            case SUSE_CREATE:
            {
                printf("Crear");
                break;
            }
            case SUSE_SCHEDULE_NEXT:
            {
                printf("Schedulear");
                break;
            }
            case SUSE_WAIT:
            {
                printf("Esperar");
                break;
            }
            case SUSE_SIGNAL:
            {
                printf("Liberar");
                break;
            }
            case SUSE_JOIN:
            {
                printf("Unir");
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