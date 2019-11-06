
#include "operaciones.h"


int sac_getattr(char* ruta ){
    sac_cli_config* _config = read_config();


    t_paquete *package = create_package(OPEN);
    add_to_package(package, (void*) ruta, strlen(ruta) + 1);

    int resultado_envio =  send_package(package, );

    return resultado_envio == -1? -1 : resultado_envio;
}

int sac_open(int socket,char* ruta ){
    t_paquete *package = create_package(OPEN);
    add_to_package(package, (void*) ruta, strlen(ruta) + 1);

    int resultado_envio =  send_package(package, socket);

    return resultado_envio == -1? -1 : resultado_envio;
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
