//
// Created by utnso on 28/10/19.
//

#ifndef TP_2019_2C_418_STATUS_CODE_SAC_SERVER_H
#define TP_2019_2C_418_STATUS_CODE_SAC_SERVER_H
#define CONFIGURACION "../sac_server.config"

#include <altaLibreria/connections.h>
#include <altaLibreria/structures.h>
#include <commons/collections/list.h>
//#include <string.h>
#include <operaciones.h>

typedef struct _sac_server_config {
    char* ip;
    int listen_port;
} sac_server_config;

void serverFunction();

sac_server_config* read_config();



#endif //TP_2019_2C_418_STATUS_CODE_SAC_SERVER_H
