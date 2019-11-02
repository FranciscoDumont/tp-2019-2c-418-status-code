#include "connections.h"

void custom_print(const char *message_template, ...) {
    fflush(stdin);
    fflush(stdout);

    va_list vargs;
    va_start(vargs, message_template);
    vprintf(message_template, vargs);
    va_end(vargs);

    fflush(stdin);
    fflush(stdout);
}

int create_socket() {
    int fd;
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        if (NETWORK_DEBUG_LEVEL >= NW_NETWORK_ERRORS) {
            custom_print("[NETWORK_ERROR][ERROR_CREATING_SOCKET]\n");
        }
        return -1;
    } else {
        int option = 1;
        setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
        if (NETWORK_DEBUG_LEVEL >= NW_ALL_DISPLAY) {
            custom_print("[NETWORK_INFO][SOCKET_CREATED_%d]\n", fd);
        }
        return fd;
    }
}

int bind_socket(int socket, int port) {
    struct sockaddr_in server;
    int bindres;

    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = INADDR_ANY;
    bzero(&(server.sin_zero), 8);

    bindres = bind(socket, (struct sockaddr *) &server,
                   sizeof(struct sockaddr));
    if (bindres != 0) {
        if (NETWORK_DEBUG_LEVEL >= NW_NETWORK_ERRORS) {
            custom_print("[NETWORK_ERROR][ERROR_BINDING_SOCKET_TO_%d]\n", port);
        }
    } else {
        if (NETWORK_DEBUG_LEVEL >= NW_ALL_DISPLAY) {
            custom_print("[NETWORK_INFO][SOCKET_BINDED_%d_TO_%d]\n", socket, port);
        }
    }
    return bindres;
}

int connect_socket(int socket, char *IP, int port) {
    struct hostent *he;
    struct sockaddr_in server;

    if ((he = gethostbyname(IP)) == NULL) {
        if (NETWORK_DEBUG_LEVEL >= NW_NETWORK_ERRORS) {
            custom_print("[NETWORK_ERROR][ERROR_FINDING_HOST_%s]\n", IP);
        }
        return -1;
    }
    if (NETWORK_DEBUG_LEVEL >= NW_ALL_DISPLAY) {
        custom_print("[NETWORK_INFO][CONNECTING_SOCKET_HOST_FOUND_%s]\n", IP);
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr = *((struct in_addr *) he->h_addr);
    bzero(&(server.sin_zero), 8);

    if (connect(socket, (struct sockaddr *) &server, sizeof(struct sockaddr)) == -1) {
        if (NETWORK_DEBUG_LEVEL >= NW_NETWORK_ERRORS) {
            custom_print("[NETWORK_ERROR][ERROR_CONNECTING_TO_%s:%d]\n", IP, port);
        }
        return -1;
    }
    if (NETWORK_DEBUG_LEVEL >= NW_ALL_DISPLAY) {
        custom_print("[NETWORK_INFO][SOCKET_CONNECTED_%d_%s:%d]\n", socket, IP, port);
    }
    return 0;
}

int close_socket(int socket) {
    close(socket);

    if (NETWORK_DEBUG_LEVEL >= NW_ALL_DISPLAY) {
        custom_print("[NETWORK_INFO][SOCKET_CLOSED_%d\n]", socket);

    }
    return 0;
}

int send_data(int destination, MessageType type, int data_size, void *data_stream) {
    MessageHeader *header = malloc(sizeof(MessageHeader));
    int sent, header_sent, data_sent;

    header->type = type;
    header->data_size = data_size;

    sent = send(destination, header, sizeof(MessageHeader), 0);
    if (sent == -1) {
        if (NETWORK_DEBUG_LEVEL >= NW_NETWORK_ERRORS) {
            custom_print("[NETWORK_ERROR][ERROR_SENDING_HEADER_TO_%d]\n", destination);
        }
        return sent;
    } else {
        if (NETWORK_DEBUG_LEVEL >= NW_ALL_DISPLAY) {
            custom_print("[NETWORK_INFO][HEADER_SENT_TO_%d_(%d_bytes)]\n", destination, sent);
        }
    }
    if (data_size > 0) {
        data_sent = send(destination, data_stream, data_size, 0);
        if (data_sent == -1) {
            if (NETWORK_DEBUG_LEVEL >= NW_NETWORK_ERRORS) {
                custom_print("[NETWORK_ERROR][ERROR_SENDING_DATA_STREAM_TO_%d]\n", destination);
            }
            return data_sent;
        } else {
            if (NETWORK_DEBUG_LEVEL >= NW_ALL_DISPLAY) {
                custom_print("[NETWORK_INFO][DATA_STREAM_SENT_TO_%d_(%d_bytes)]\n", destination,
                             data_sent);
            }
            sent += data_sent;
        }
    } else {
        if (NETWORK_DEBUG_LEVEL >= NW_ALL_DISPLAY) {
            custom_print("[NETWORK_INFO][NO_STREAM_TO_SEND]\n", destination);
        }
    }

    return sent;
}

int receive_header(int source, MessageHeader *buffer) {
    int rec;
    rec = recv(source, buffer, sizeof(MessageHeader), 0);
    if (rec > 0) {
        if (NETWORK_DEBUG_LEVEL >= NW_ALL_DISPLAY) {
            custom_print("[NETWORK_INFO][HEADER_RECIEVED_FROM_%d_(%d_bytes)]\n", source, rec);
        }
    } else {
        if (NETWORK_DEBUG_LEVEL >= NW_NETWORK_ERRORS) {
            custom_print("[NETWORK_ERROR][ERROR_RECIEVING_HEADER_FROM_%d]\n", source);
        }
    }
    return rec;
}

int receive_data(int source, void *buffer, int data_size) {
    int rec;
    rec = recv(source, buffer, data_size, 0);
    if (rec > 0) {
        if (NETWORK_DEBUG_LEVEL >= NW_ALL_DISPLAY) {
            custom_print("[NETWORK_INFO][DATA_RECIEVED_FROM_%d_(%d_bytes)]\n", source, rec);
        }
    } else {
        if (NETWORK_DEBUG_LEVEL >= NW_NETWORK_ERRORS) {
            custom_print("[NETWORK_ERROR][ERROR_RECIEVING_DATA_FROM_%d]\n", source);
        }
    }
    return rec;
}

//Funcion que va a correr el hilo del servidor cuando haya una nueva conexion
void* server_client(void* _params){

    //casteo los parametros porque a pthread hay que pasarle un puntero a void
    t_thread_client* params = (t_thread_client* ) _params;
    MessageHeader* header = malloc(sizeof(MessageHeader));

    int datos_recividos  = recv(params->socket, header, sizeof(MessageHeader),0);

    while (datos_recividos != -1) {

        if(datos_recividos == 0){
            //si devuelve 0 es porque se corto la conexion
            params->lost_connection(params->socket,params->client_ip, params->connection_port);
            close(params->socket);
            break;

        }else {

            params->incoming_message(params->socket, params->client_ip, params->connection_port, header);

        }

        datos_recividos  = recv(params->socket, header, sizeof(MessageHeader),0);
    }

    //Libero la memoria que pedi
    free(header);
    free(params->client_ip);
    free(params);

}

int start_multithread_server(int socket,
                             void (*new_connection)(int fd, char *ip, int port),
                             void (*lost_connection)(int fd, char *ip, int port),
                             void (*incoming_message)(int fd, char *ip, int port, MessageHeader *header)) {

    int addrlen, new_socket, i, bytesread, sd;
    struct sockaddr_in address;

    //Es header del mensaje del cliente
    MessageHeader *incoming;

    //Empieza a escuchar el socket y puede tener peticiones pendientes
    if (listen(socket, MAX_CONN) < 0) {

        return -1;
    }
    addrlen = sizeof(address);

    while (1) {

        //Se fija si hay nuevas conexion
        if ((new_socket = accept(socket, (struct sockaddr *) &address, (socklen_t *) &addrlen)) > 0) {

            //Acepta la nueva conexion
            new_connection(new_socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));

            getpeername(new_socket, (struct sockaddr *) &address, (socklen_t *) &addrlen);

            //Creo los parametros de la funcion que le voy a pasar al hilo
            t_thread_client *parametros_cliente = malloc(sizeof(t_thread_client));

            parametros_cliente->socket = new_socket;
            parametros_cliente->client_ip = inet_ntoa(address.sin_addr);
            parametros_cliente->connection_port = ntohs(address.sin_port);
            parametros_cliente->incoming_message = incoming_message;
            parametros_cliente->lost_connection = lost_connection;

            //Inicializo el hilo
            pthread_t thread_client;

            //Lo creo y pongo en ejecucion
            pthread_create(&thread_client, NULL, server_client, (void *) parametros_cliente);
            pthread_detach(thread_client);
        }

    }
}

int start_server(int socket,
                 void (*new_connection)(int fd, char * ip, int port),
                 void (*lost_connection)(int fd, char * ip, int port),
                 void (*incoming_message)(int fd, char * ip, int port, MessageHeader * header)) {

    int addrlen, new_socket ,client_socket_array[MAX_CONN], activity, i, bytesread, sd;
    int max_sd;
    struct sockaddr_in address;
    fd_set readfds;

    MessageHeader * incoming;

    for (i = 0; i < MAX_CONN; i++) {
        client_socket_array[i] = 0;
    }

    if (listen(socket, MAX_CONN) < 0) {
        if(NETWORK_DEBUG_LEVEL >= NW_NETWORK_ERRORS) {
            custom_print("[NETWORK_ERROR][ERROR_LISTENING_FD_%d]\n", socket);
        }
        return -1;
    }
    if(NETWORK_DEBUG_LEVEL >= NW_ALL_DISPLAY) {
        custom_print("[NETWORK_INFO][SOCKET_NOW_LISTENING_%d]\n", socket);
    }

    addrlen = sizeof(address);

    while(1) {
        FD_ZERO(&readfds);

        FD_SET(socket, &readfds);
        max_sd = socket;

        for (i = 0 ; i < MAX_CONN ; i++) {
            sd = client_socket_array[i];
            if (sd > 0){
                FD_SET( sd , &readfds);
            }
            if (sd > max_sd){
                max_sd = sd;
            }
        }

        if(NETWORK_DEBUG_LEVEL >= NW_ALL_DISPLAY) {
            custom_print("[NETWORK_INFO][SOCKET_AWAITING_CONNECTION_%d]\n", socket);
        }
        activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

        if (activity < 0) {
            if(NETWORK_DEBUG_LEVEL >= NW_NETWORK_ERRORS) {
                custom_print("[NETWORK_ERROR][ERROR_SELECTING_FD_%d]\n", socket);
            }
        }

        if (FD_ISSET(socket, &readfds)) {
            if ((new_socket = accept(socket,
                                     (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
                if(NETWORK_DEBUG_LEVEL >= NW_NETWORK_ERRORS) {
                    custom_print("[NETWORK_ERROR][ERROR_ACCEPTING_SOCKET_%d_%s:%d]\n",
                                 new_socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));
                }
            }
            if(NETWORK_DEBUG_LEVEL >= NW_ALL_DISPLAY) {
                custom_print("[NETWORK_INFO][NEW_CONNECTION_%d_%d_%s:%d]\n", socket, new_socket,
                             inet_ntoa(address.sin_addr), ntohs(address.sin_port));
            }
            new_connection(new_socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));

            for (i = 0; i < MAX_CONN; i++) {
                if (client_socket_array[i] == 0) {
                    client_socket_array[i] = new_socket;
                    break;
                }
            }
        }

        for (i = 0; i < MAX_CONN; i++) {
            sd = client_socket_array[i];

            if (FD_ISSET(sd, &readfds)) {
                int client_socket = sd;

                incoming = malloc(sizeof(MessageHeader));

                if ((read(client_socket, incoming, sizeof(MessageHeader))) <= 0) {
                    getpeername(sd , (struct sockaddr*)&address , (socklen_t*)&addrlen);

                    if(NETWORK_DEBUG_LEVEL >= NW_ALL_DISPLAY) {
                        custom_print("[NETWORK_INFO][LOST_CONNECTION_%d_%d_%s:%d]\n", socket, client_socket,
                                     inet_ntoa(address.sin_addr), ntohs(address.sin_port));
                    }
                    lost_connection(client_socket, inet_ntoa(address.sin_addr) , ntohs(address.sin_port));

                    close(sd);
                    client_socket_array[i] = 0;
                } else {

                    if(NETWORK_DEBUG_LEVEL >= NW_ALL_DISPLAY) {
                        custom_print("[NETWORK_INFO][INCOMING_MESSAGE_%d_%d]\n", socket, client_socket);
                    }
                    incoming_message(client_socket, inet_ntoa(address.sin_addr) , ntohs(address.sin_port), incoming);

                }

                free(incoming);
            }
        }
    }
}


// Serializacion
t_paquete * create_package(MessageType tipo) {
    t_paquete *paquete = malloc(sizeof(t_paquete));
    paquete->header = (MessageHeader *) malloc(sizeof(MessageHeader));
    paquete->header->type = tipo;
    paquete->header->data_size = 0;
    paquete->stream = NULL;
    return paquete;
}

void add_to_package(t_paquete *paquete, void *valor, int tamanio) {
    //valor = "hola"
    //tamanio = 5(4 del "hola" mas caracter terminador)
    //paquete->stream == NULL
    //paquete->stream length = 0
    //paquete->header->data_size == 0
    paquete->stream = realloc(paquete->stream, paquete->header->data_size + tamanio + sizeof(int));
    //paquete->stream length == 0 + 5 + 1

    memcpy(paquete->stream + paquete->header->data_size, &tamanio, sizeof(int));
    //paquete->stream == 5
    memcpy(paquete->stream + paquete->header->data_size + sizeof(int), valor, tamanio);
    //paquete->stream == 5hola

    paquete->header->data_size += tamanio + sizeof(int);
    //paquete->header->data_size == 6
}

int send_package(t_paquete *paquete, int socket_cliente) {
    int bytes = paquete->header->data_size + 2 * sizeof(int);
    void *a_enviar = serialize_package(paquete, bytes);

    int sent;

    if ((sent = send(socket_cliente, a_enviar, bytes, 0)) == -1) {
        printf("Error en el envio");
    }

    free(a_enviar);
    return sent;
}

void *serialize_package(t_paquete *paquete, int bytes) {
    void *serialized = malloc(bytes);
    int desplazamiento = 0;

    memcpy(serialized + desplazamiento, &(paquete->header->type), sizeof(int));
    desplazamiento += sizeof(int);
    memcpy(serialized + desplazamiento, &(paquete->header->data_size), sizeof(int));
    desplazamiento += sizeof(int);
    memcpy(serialized + desplazamiento, paquete->stream, paquete->header->data_size);

    return serialized;
}

void free_package(t_paquete *paquete) {
//	free(paquete->header->type);
    free(paquete->header);
    free(paquete->stream);
    free(paquete);
}

t_list *receive_package(int socket_cliente, MessageHeader *header) {
    int size = header->data_size;
    int desplazamiento = 0;
    void *buffer = malloc(size);
    recv(socket_cliente, buffer, size, MSG_WAITALL);
    t_list *valores = list_create();
    int tamanio;

    while (desplazamiento < size) {
        memcpy(&tamanio, buffer + desplazamiento, sizeof(int));
        desplazamiento += sizeof(int);
        char *valor = malloc(tamanio);
        memcpy(valor, buffer + desplazamiento, tamanio);
        desplazamiento += tamanio;
        list_add(valores, valor);
    }

    free(buffer);
    return valores;
}