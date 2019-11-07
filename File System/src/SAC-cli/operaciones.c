
#include "operaciones.h"

int sac_getattr(const char *ruta, struct stat *stbuf){

    t_paquete *package = create_package(GETATTR);
    add_to_package(package, (void*) ruta, strlen(ruta) + 1);

    int resultado_envio =  send_package(package,server_socket);

    int res = 0;

    memset(stbuf, 0, sizeof(struct stat));

    //Si path es igual a "/" nos estan pidiendo los atributos del punto de montaje

    if (strcmp(ruta, "/") == 0) {
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
    } else if (strcmp(ruta, DEFAULT_FILE_PATH) == 0) {
        stbuf->st_mode = S_IFREG | 0444;
        stbuf->st_nlink = 1;
        stbuf->st_size = strlen(DEFAULT_FILE_CONTENT);
    } else {
        res = -ENOENT;
    }


    return res;
}

int sac_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {

    t_paquete *package = create_package(READDIR);
    add_to_package(package, (void*) path, strlen(path) + 1);

    int resultado_envio =  send_package(package, server_socket);


    (void) offset;
    (void) fi;

    if (strcmp(path, "/") != 0)
        return -ENOENT;

    // "." y ".." son entradas validas, la primera es una referencia al directorio donde estamos parados
    // y la segunda indica el directorio padre
    filler(buf, ".", NULL, 0);
    filler(buf, "..", NULL, 0);
    filler(buf, DEFAULT_FILE_NAME, NULL, 0);

    return 0;
}


int sac_open(const char *ruta, struct fuse_file_info *fi){
    t_paquete *package = create_package(OPEN);
    add_to_package(package, (void*) ruta, strlen(ruta) + 1);

    int resultado_envio =  send_package(package, server_socket);

    if (strcmp(ruta, DEFAULT_FILE_PATH) != 0)
        return -ENOENT;

    if ((fi->flags & 3) != O_RDONLY)
        return -EACCES;

    return 0;
}

int sac_mkdir(char* ruta){
    t_paquete *package = create_package(MKDIR);
    add_to_package(package, (void*) ruta, strlen(ruta) + 1);

    int resultado_envio =  send_package(package, server_socket);

    return resultado_envio == -1? -1 : resultado_envio;
}

int sac_mknod(char* ruta){
    t_paquete *package = create_package(MKNOD);
    add_to_package(package, (void*) ruta, strlen(ruta) + 1);

    int resultado_envio =  send_package(package, server_socket);

    return resultado_envio == -1? -1 : resultado_envio;
}

int sac_read(const char *ruta, char *buf, size_t size, off_t offset, struct fuse_file_info *fi){
    t_paquete *package = create_package(READ);
    add_to_package(package, (void*) ruta, strlen(ruta) + 1);

    int resultado_envio =  send_package(package, server_socket);

    size_t len;
    (void) fi;
    if (strcmp(ruta, DEFAULT_FILE_PATH) != 0)
        return -ENOENT;

    len = strlen(DEFAULT_FILE_CONTENT);
    if (offset < len) {
        if (offset + size > len)
            size = len - offset;
        memcpy(buf, DEFAULT_FILE_CONTENT + offset, size);
    } else
        size = 0;

    return size;
}

int sac_write (const char * ruta, const char * algo, size_t size, off_t off_set, struct fuse_file_info * nosequees){
    t_paquete *package = create_package(WRITE);
    add_to_package(package, (void*) ruta, strlen(ruta) + 1);

    int resultado_envio =  send_package(package, server_socket);

    return resultado_envio == -1? -1 : resultado_envio;
}
