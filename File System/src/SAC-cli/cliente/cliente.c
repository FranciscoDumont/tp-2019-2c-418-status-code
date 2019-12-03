
#include "cliente.h"

int socket_conexion;

sac_cli_config* read_config(){
    sac_cli_config* config;
    t_log* logger;

    config = malloc(sizeof(sac_cli_config));
    config->talk_port = malloc(30);

    logger = log_create("../sac_cli.log", "sac-cli", 1, LOG_LEVEL_TRACE);


    t_config* config_file = config_create("../sac_cli.config");
    config->talk_port = config_get_int_value(config_file, "LISTEN_PORT");
    config->ip = config_get_string_value(config_file, "IP");

    return config;
}


int* sac_open(char* ruta ){
    //Envia la primera peticion
    t_paquete *package = create_package(OPEN);
    add_to_package(package, (void*) ruta, strlen(ruta) + 1);
    send_package(package, socket_conexion);

    //Recive la respuesta
    MessageHeader* header_respuesta;
    receive_header( socket_conexion, header_respuesta);
    t_list* elementos_respuesta = receive_package(socket_conexion, header_respuesta);

    return (int*) list_get( elementos_respuesta, 0);
}


int sac_mkdir(int socket,char* ruta){
    t_paquete *package = create_package(MKDIR);
    add_to_package(package, (void*) ruta, strlen(ruta) + 1);

    int resultado_envio =  send_package(package, socket);

    return resultado_envio == -1? -1 : resultado_envio;
}

int sac_mknode(int socket,char* ruta){
    t_paquete *package = create_package(MKNOD);
    add_to_package(package, (void*) ruta, strlen(ruta) + 1);

    int resultado_envio =  send_package(package, socket);

    return resultado_envio == -1? -1 : resultado_envio;
}

int sac_read(int socket,char* ruta ){
    t_paquete *package = create_package(READ);
    add_to_package(package, (void*) ruta, strlen(ruta) + 1);

    int resultado_envio =  send_package(package, socket);

    return resultado_envio == -1? -1 : resultado_envio;
}

int sac_write(int socket,char* ruta ){
    t_paquete *package = create_package(WRITE);
    add_to_package(package, (void*) ruta, strlen(ruta) + 1);

    int resultado_envio =  send_package(package, socket);

    return resultado_envio == -1? -1 : resultado_envio;
}


int main(){
    //Creo un socket
    socket_conexion = create_socket();
    sac_cli_config* config = read_config();

    char* mensaje = malloc(20);

    //conecto el socket
    if(-1 == connect_socket(socket_conexion, config->ip, config->talk_port)){
        printf("Error connect ::NOT FOUND\n");
    }else {
        printf("EL connect anda bien ::E \n");
    }

    int* respuesta = sac_open("/Carpeta1/algo");

    printf("Respuesta es:%d\n", *respuesta);


    //Libero el socket
    close_socket(socket_conexion);


    return 0;
}