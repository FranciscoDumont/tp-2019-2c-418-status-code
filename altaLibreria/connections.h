#ifndef ALTALIBRERIA_CONNECTIONS_H_
#define ALTALIBRERIA_CONNECTIONS_H_

#define MAX_CONN 40
#define null NULL

#include <arpa/inet.h>
#include <commons/collections/list.h>
#include <commons/bitarray.h>
#include <commons/config.h>
#include <commons/log.h>
#include <commons/string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <limits.h>
#include <dirent.h>
#include <pthread.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <ctype.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "structures.h"

int client_socket_array[MAX_CONN];

typedef struct t_thread_client {

	int socket;
	char * client_ip;
	int connection_port;
	void (*lost_connection)(int, char*, int);
	void (*incoming_message)(int, char*, int,MessageHeader*);

}t_thread_client;


void custom_print(const char* message_template, ...);
int create_socket();
int bind_socket(int socket, int port);
int connect_socket(int socket, char * IP, int port);
int close_socket(int socket);
int send_data(int destination, MessageType type, int data_size, void * data_stream);
int receive_header(int source, MessageHeader * buffer);
int receive_data(int source, void * buffer, int data_size);
int start_server(int socket,
				 void (*new_connection)(int fd, char * ip, int port),
				 void (*lost_connection)(int fd, char * ip, int port),
				 void (*incoming_message)(int fd, char * ip, int port, MessageHeader * header));

t_paquete* create_package(MessageType tipo);
void add_to_package(t_paquete* paquete, void* valor, int tamanio);
int send_package(t_paquete* paquete, int socket_cliente);
void* serialize_package(t_paquete* paquete, int bytes);
void free_package(t_paquete* paquete);
t_list* receive_package(int socket_cliente, MessageHeader *header);
void* server_client(void* params);
int start_multithread_server(int socket,
							 void (*new_connection)(int fd, char *ip, int port),
							 void (*lost_connection)(int fd, char *ip, int port),
							 void (*incoming_message)(int fd, char *ip, int port, MessageHeader *header));


#endif
