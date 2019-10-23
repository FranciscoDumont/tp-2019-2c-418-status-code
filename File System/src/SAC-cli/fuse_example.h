//
// Created by utnso on 23/10/19.
//

#ifndef TP_2019_2C_418_STATUS_CODE_FUSE_EXAMPLE_H
#define TP_2019_2C_418_STATUS_CODE_FUSE_EXAMPLE_H

#include <stddef.h>
#include <stdlib.h>
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include "fuse_example.h"
#include <altaLibreria/connections.h>
#include <altaLibreria/structures.h>
#define PORT 8080

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
 * Esta es una estructura auxiliar utilizada para almacenar parametros
 * que nosotros le pasemos por linea de comando a la funcion principal
 * de FUSE
 */
struct t_runtime_options {
    char* welcome_msg;
} runtime_options;

/*
 * Esta Macro sirve para definir nuestros propios parametros que queremos que
 * FUSE interprete. Esta va a ser utilizada mas abajo para completar el campos
 * welcome_msg de la variable runtime_options
 */
#define CUSTOM_FUSE_OPT_KEY(t, p, v) { t, offsetof(struct t_runtime_options, p), v }


/*
 * @DESC
 *  Esta función va a ser llamada cuando a la biblioteca de FUSE le llege un pedido
 * para obtener la metadata de un archivo/directorio. Esto puede ser tamaño, tipo,
 * permisos, dueño, etc ...
 *
 * @PARAMETROS
 * 		path - El path es relativo al punto de montaje y es la forma mediante la cual debemos
 * 		       encontrar el archivo o directorio que nos solicitan
 * 		stbuf - Esta esta estructura es la que debemos completar
 *
 * 	@RETURN
 * 		O archivo/directorio fue encontrado. -ENOENT archivo/directorio no encontrado
 */
static int hello_getattr(const char *path, struct stat *stbuf) {
    int res = 0;

    memset(stbuf, 0, sizeof(struct stat));

    //Si path es igual a "/" nos estan pidiendo los atributos del punto de montaje

    if (strcmp(path, "/") == 0) {
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
    } else if (strcmp(path, DEFAULT_FILE_PATH) == 0) {
        stbuf->st_mode = S_IFREG | 0444;
        stbuf->st_nlink = 1;
        stbuf->st_size = strlen(DEFAULT_FILE_CONTENT);
    } else {
        res = -ENOENT;
    }
    printf("hello_getattr\n");
    return res;
}

static int example_fopen(const char *path, struct stat *stbuf){
    printf("example_fopen\n");
    return 0;
}



/*
 * @DESC
 *  Esta función va a ser llamada cuando a la biblioteca de FUSE le llege un pedido
 * para obtener la lista de archivos o directorios que se encuentra dentro de un directorio
 *
 * @PARAMETROS
 * 		path - El path es relativo al punto de montaje y es la forma mediante la cual debemos
 * 		       encontrar el archivo o directorio que nos solicitan
 * 		buf - Este es un buffer donde se colocaran los nombres de los archivos y directorios
 * 		      que esten dentro del directorio indicado por el path
 * 		filler - Este es un puntero a una función, la cual sabe como guardar una cadena dentro
 * 		         del campo buf
 *
 * 	@RETURN
 * 		O directorio fue encontrado. -ENOENT directorio no encontrado
 */
static int hello_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
    (void) offset;
    (void) fi;

    if (strcmp(path, "/") != 0)
        return -ENOENT;

    // "." y ".." son entradas validas, la primera es una referencia al directorio donde estamos parados
    // y la segunda indica el directorio padre
    filler(buf, ".", NULL, 0);
    filler(buf, "..", NULL, 0);
    filler(buf, DEFAULT_FILE_NAME, NULL, 0);
    printf("hello_readdir\n");
    return 0;
}

static int example_mknod(const char *path, mode_t mode, dev_t rdev)
{
    int res;

    res = mknod(path, mode, rdev);
    if(res == -1)
        return -errno;

    printf("mknod\n");
    return 0;
}

static int example_utimens(const char *path, const struct timespec ts[2])
{
    int res;

    res = utimensat(0, path, ts, AT_SYMLINK_NOFOLLOW);
    if (res == -1)
        return -errno;

    return 0;
}

/*
 * @DESC
 *  Esta función va a ser llamada cuando a la biblioteca de FUSE le llege un pedido
 * para tratar de abrir un archivo
 *
 * @PARAMETROS
 * 		path - El path es relativo al punto de montaje y es la forma mediante la cual debemos
 * 		       encontrar el archivo o directorio que nos solicitan
 * 		fi - es una estructura que contiene la metadata del archivo indicado en el path
 *
 * 	@RETURN
 * 		O archivo fue encontrado. -EACCES archivo no es accesible
 */
static int example_open(const char *path, struct fuse_file_info *fi) {
    if (strcmp(path, DEFAULT_FILE_PATH) != 0)
        return -ENOENT;

    if ((fi->flags & 3) != O_RDONLY)
        return -EACCES;

    printf("example_open\n");
    return 0;
}

/*
 * @DESC
 *  Esta función va a ser llamada cuando a la biblioteca de FUSE le llege un pedido
 * para obtener el contenido de un archivo
 *
 * @PARAMETROS
 * 		path - El path es relativo al punto de montaje y es la forma mediante la cual debemos
 * 		       encontrar el archivo o directorio que nos solicitan
 * 		buf - Este es el buffer donde se va a guardar el contenido solicitado
 * 		size - Nos indica cuanto tenemos que leer
 * 		offset - A partir de que posicion del archivo tenemos que leer
 *
 * 	@RETURN
 * 		Si se usa el parametro direct_io los valores de retorno son 0 si  elarchivo fue encontrado
 * 		o -ENOENT si ocurrio un error. Si el parametro direct_io no esta presente se retorna
 * 		la cantidad de bytes leidos o -ENOENT si ocurrio un error. ( Este comportamiento es igual
 * 		para la funcion write )
 */
static int hello_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    size_t len;
    (void) fi;
    if (strcmp(path, DEFAULT_FILE_PATH) != 0)
        return -ENOENT;

    len = strlen(DEFAULT_FILE_CONTENT);
    if (offset < len) {
        if (offset + size > len)
            size = len - offset;
        memcpy(buf, DEFAULT_FILE_CONTENT + offset, size);
    } else
        size = 0;

    printf("hello_read\n");
    return 17;
}


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