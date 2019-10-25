
#ifndef TP_2019_2C_418_STATUS_CODE_FUSE_EXAMPLE_H_
#define TP_2019_2C_418_STATUS_CODE_FUSE_EXAMPLE_H_
#include "operaciones.h"

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
 */

static struct fuse_operations hello_oper = {
        .getattr = hello_getattr,
        .readdir = hello_readdir,
        .open = example_open,
        .read = hello_read,
        .mknod = example_mknod,
        .utimens = example_utimens
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
        // Este es un parametro definido por nosotros
        CUSTOM_FUSE_OPT_KEY("--welcome-msg %s", welcome_msg, 0),

        // Estos son parametros por defecto que ya tiene FUSE
        FUSE_OPT_KEY("-V", KEY_VERSION),
        FUSE_OPT_KEY("--version", KEY_VERSION),
        FUSE_OPT_KEY("-h", KEY_HELP),
        FUSE_OPT_KEY("--help", KEY_HELP),
        FUSE_OPT_END,
};

#endif //TP_2019_2C_418_STATUS_CODE_FUSE_EXAMPLE_H
