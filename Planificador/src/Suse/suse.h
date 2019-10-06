#ifndef SUSE_H
#define SUSE_H

#include <stdio.h>
#include <string.h>
#include <pthread.h>

#include <commons/config.h>
#include <commons/log.h>
#include <altaLibreria/connections.h>
#include <altaLibreria/structures.h>
#include "suseStructures.h"

t_log* iniciar_log();
void leerConfig();
void* server_function(void* arg);
void suse_init(int fd, char * ip, int port, MessageHeader * headerStruct);
void suse_create(int fd, char * ip, int port, MessageHeader * headerStruct);
void suse_schedule_next(int fd, char * ip, int port, MessageHeader * headerStruct);
void suse_wait(int fd, char * ip, int port, MessageHeader * headerStruct);
void suse_signal(int fd, char * ip, int port, MessageHeader * headerStruct);
void suse_join(int fd, char * ip, int port, MessageHeader * headerStruct);

#endif //SUSE_H
