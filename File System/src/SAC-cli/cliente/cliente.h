//
// Created by utnso on 02/11/19.
//

#ifndef SAC_CLI_CLIENTE_H
#define SAC_CLI_CLIENTE_H

#include <altaLibreria/connections.h>
#include <altaLibreria/structures.h>
#include <commons/collections/list.h>
#include <string.h>

typedef struct _sac_cli_config {
    char* ip;
    int talk_port;
} sac_cli_config;


#endif //SAC_CLI_CLIENTE_H
