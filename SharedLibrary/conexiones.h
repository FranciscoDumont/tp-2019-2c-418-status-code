//
// Created by utnso on 25/09/19.
//

#ifndef TP_2019_2C_418_STATUS_CODE_CONEXIONES_H
#define TP_2019_2C_418_STATUS_CODE_CONEXIONES_H

#endif //TP_2019_2C_418_STATUS_CODE_CONEXIONES_H

#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<unistd.h>
#include<sys/socket.h>
#include<netdb.h>
#include<string.h>


typedef enum
{
    MENSAJE,
    PAQUETE
}op_code;

typedef struct
{
    int size;
    void* stream;
} t_buffer;

typedef struct
{
    op_code codigo_operacion;
    t_buffer* buffer;
} t_paquete;