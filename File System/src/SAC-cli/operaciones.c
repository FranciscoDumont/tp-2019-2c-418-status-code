
#include "operaciones.h"

//
// int hello_getattr(const char *path, struct stat *stbuf) {
//
//}
//    t_paquete *package = create_package(ABC);
//    add_to_package(package,(void*) path, strlen(path) + 1);
//    add_to_package(package,(void*) stbuf, sizeof(stat));
//
//    if(send_package(package, server_socket) == -1){
//        free_package(package);
//        printf("Error sending: %s\n", path);
//        return -1;
//    }else {
//        free_package(package);
//        return 0;
//    }

//}

// int example_fopen(const char *path, struct stat *stbuf){
//    return 0;
//}
//
// int hello_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
//     return 0;
//}
//
// int example_mknod(const char *path, mode_t mode, dev_t rdev){
//    return 0;
//}
//
// int example_utimens(const char *path, const struct timespec ts[2]){
//    return 0;
//}
//
// int example_open(const char *path, struct fuse_file_info *fi) {
//     return 0;
//}
//
// int hello_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
//     return 0;
//}

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
