//
// Created by utnso on 05/10/19.
//
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>

#ifndef TP_2019_2C_418_STATUS_CODE_MUSE_H
#define TP_2019_2C_418_STATUS_CODE_MUSE_H

// Configuraciones
typedef struct MEMConfig{
    int listen_port;
    int memory_size;
    int page_size;
    int swap_size;
} MEMConfig;

t_config * config_file;
MEMConfig config;

void read_memory_config();


#endif //TP_2019_2C_418_STATUS_CODE_MUSE_H
