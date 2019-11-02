#ifndef TP_2019_2C_418_STATUS_CODE_FUSE_EXAMPLE_H_
#define TP_2019_2C_418_STATUS_CODE_FUSE_EXAMPLE_H_

#include <altaLibreria/connections.h>
#include <altaLibreria/structures.h>
#include <commons/collections/list.h>
#include <stddef.h>
#include <stdlib.h>
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

#define DEFAULT_FILE_PATH "/"
#define DEFAULT_FILE_CONTENT "Hello World!\n"
#define DEFAULT_FILE_NAME "hello"
/*
 * Esta es una estructura auxiliar utilizada para almacenar parametros
 * que nosotros le pasemos por linea de comando a la funcion principal
 * de FUSE
 */

struct t_runtime_options {
    char* welcome_msg;
} runtime_options;

/*
 * Esta es la estructura principal de FUSE con la cual nosotros le decimos a
 * biblioteca que funciones tiene que invocar segun que se le pida a FUSE.
 * Como se observa la estructura contiene punteros a funciones.
 *
*/
typedef struct _sac_cli_config {
    char* ip;
    int talk_port;
} sac_cli_config;

//DECLARACIONES DE FUNCIONES
sac_cli_config* read_config();
void sac_init();
int sac_getattr(const char *ruta, struct stat *stbuf);
int sac_open(const char *ruta, struct fuse_file_info *fi);
int sac_mkdir(char* ruta);
int sac_mknod(char* ruta);
int sac_read(const char *ruta, char *buf, size_t size, off_t offset, struct fuse_file_info *fi);
int sac_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi);
int sac_write (const char *, const char *, size_t, off_t, struct fuse_file_info *);

static struct fuse_operations hello_oper = {
        .getattr = sac_getattr,
        .open = sac_open,
        .read = sac_read,
        .readdir = sac_readdir,
        .mknod = sac_mknod,
        .mkdir = sac_mkdir,
        .write = sac_write
};


/** keys for FUSE_OPT_ options */
enum {
    KEY_VERSION,
    KEY_HELP,
};

/*
 * Esta estructura es utilizada para decirle a la biblioteca de FUSE que
 * parametro puede recibir y donde tiene que guardar el valor de estos
 */
static struct fuse_opt fuse_options[] = {

        // Estos son parametros por defecto que ya tiene FUSE
        FUSE_OPT_KEY("-V", KEY_VERSION),
        FUSE_OPT_KEY("--version", KEY_VERSION),
        FUSE_OPT_KEY("-h", KEY_HELP),
        FUSE_OPT_KEY("--help", KEY_HELP),
        FUSE_OPT_END,
};


#endif //TP_2019_2C_418_STATUS_CODE_FUSE_EXAMPLE_H
