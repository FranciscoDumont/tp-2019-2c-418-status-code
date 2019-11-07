#ifndef TP_2019_2C_418_STATUS_CODE_FUSE_EXAMPLE_H_
#define TP_2019_2C_418_STATUS_CODE_FUSE_EXAMPLE_H_

#include "operaciones.h"

#define DEFAULT_FILE_PATH "/"
#define DEFAULT_FILE_CONTENT "Hello World!\n"
#define DEFAULT_FILE_NAME "hello"
/*
 * Esta es una estructura auxiliar utilizada para almacenar parametros
 * que nosotros le pasemos por linea de comando a la funcion principal
 * de FUSE
 */

extern int server_socket;

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
}sac_cli_config;


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
