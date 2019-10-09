//
// Created by utnso on 05/10/19.
//

#ifndef SUSE_SUSESTRUCTURES_H
#define SUSE_SUSESTRUCTURES_H

typedef struct _SUSEConfig {
    int listen_port;
    int metrics_timer;
    int max_multiprog;
    char** sem_ids;
    char** sem_init;
    char** sem_max;
    double alpha_sjf;
} SUSEConfig;

typedef struct _t_newProgramData{
    char* ip;
    int port;
} t_newProgramData;

typedef struct _t_thread{
    int identificador; //TID?
    char* pid; //identificador de programa
} t_thread;

typedef struct _t_programa{
    char* identificador; //IP::PORT
    t_list* ready; //Thread list
    t_thread* exec;
} t_programa;

typedef struct _t_new_comm{
    int fd;
    char* ip;
    int port;
    t_list* received;
} t_new_comm;

#endif //SUSE_SUSESTRUCTURES_H
