//
// Created by utnso on 03/10/19.
//

#ifndef TP_2019_2C_418_STATUS_CODE_FUNCIONES_H
#define TP_2019_2C_418_STATUS_CODE_FUNCIONES_H

static struct fuse_operations fuse_operations_implemented = {
        .getattr = callback_getattr, // Obtiene la metadata de un archivo o directorio
        .readdir = callback_readdir, // Obtiene una lista de archivos o directorios
        .open = callback_open, //Abre un archivo
        .read = callback_read, //Obtiene el contenido de un archivo
        //Definir todas las operaciones que necesite FUSE para funcionar por ejemplo las pedidas en los casos de prueba
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

struct t_runtime_options {
    char* welcome_msg;
} runtime_options;

/*
 * Esta Macro sirve para definir nuestros propios parametros que queremos que
 * FUSE interprete. Esta va a ser utilizada mas abajo para completar el campos
 * welcome_msg de la variable runtime_options
 */
#define CUSTOM_FUSE_OPT_KEY(t, p, v) { t, offsetof(struct t_runtime_options, p), v }



#endif //TP_2019_2C_418_STATUS_CODE_FUNCIONES_H
