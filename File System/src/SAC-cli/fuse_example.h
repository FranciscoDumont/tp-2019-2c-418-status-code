
#ifndef TP_2019_2C_418_STATUS_CODE_FUSE_EXAMPLE_H_
#define TP_2019_2C_418_STATUS_CODE_FUSE_EXAMPLE_H_

#include <stddef.h>
#include <stdlib.h>
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <altaLibreria/connections.h>
#include <altaLibreria/structures.h>
#include "operaciones.h"

/* Este es el contenido por defecto que va a contener
* el unico archivo que se encuentre presente en el FS.
* Si se modifica la cadena se podra ver reflejado cuando
* se lea el contenido del archivo
*/
#define DEFAULT_FILE_CONTENT "Hello World!\n"

/*
 * Este es el nombre del archivo que se va a encontrar dentro de nuestro FS
 */
#define DEFAULT_FILE_NAME "hello"

/*
 * Este es el path de nuestro, relativo al punto de montaje, archivo dentro del FS
 */
#define DEFAULT_FILE_PATH "/" DEFAULT_FILE_NAME



/*
 * Esta Macro sirve para definir nuestros propios parametros que queremos que
 * FUSE interprete. Esta va a ser utilizada mas abajo para completar el campos
 * welcome_msg de la variable runtime_options
 */
#define CUSTOM_FUSE_OPT_KEY(t, p, v) { t, offsetof(struct t_runtime_options, p), v }

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
