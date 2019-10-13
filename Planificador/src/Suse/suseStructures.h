#ifndef SUSE_SUSESTRUCTURES_H
#define SUSE_SUSESTRUCTURES_H

typedef int TID;
typedef char* PID;

typedef struct _SUSEConfig {
    int listen_port;
    int metrics_timer;
    int max_multiprog;
    char** sem_ids;
    char** sem_init;
    char** sem_max;
    double alpha_sjf;
} SUSEConfig;

typedef struct _t_thread{
    TID tid;
    PID pid;
    t_list* exec_list; //Lista de intervals
    t_list* ready_list; //Lista de intervals
    struct timespec start_time;
} t_thread;

typedef struct _t_programa{
    PID pid; //IP::PORT
    t_list* ready; //Thread list
    t_thread* exec;
    bool executing;
} t_programa;

typedef struct _t_new_comm{
    int fd;
    char* ip;
    int port;
    t_list* received;
} t_new_comm;

typedef struct _t_new_response{
    int fd;
    int response;
    MessageType header;
} t_new_response;

typedef struct _interval{
    struct timespec start_time;
    struct timespec end_time;
} interval;

#endif //SUSE_SUSESTRUCTURES_H