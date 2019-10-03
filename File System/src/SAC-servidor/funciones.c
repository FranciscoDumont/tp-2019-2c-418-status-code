#include "funciones.h"
#include <stddef.h>
#include <stdlib.h>
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
/*
*       @PARAMETROS
* 		path - El path es relativo al punto de montaje y es la forma mediante la cual debemos
* 		       encontrar el archivo o directorio que nos solicitan
* 		stbuf - Esta esta estructura es la que debemos completar
*
*       En todos los callback implementados hasta el momento si llega a retornan error devuelve -ENOENT
*/


static int callback_getattr(const char *path, struct stat *stbuf) {
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
    return res;
}

/*
 *   @PARAMETROS
 * 		path - El path es relativo al punto de montaje y es la forma mediante la cual debemos
 * 		       encontrar el archivo o directorio que nos solicitan
 * 		buf - Este es un buffer donde se colocaran los nombres de los archivos y directorios
 * 		      que esten dentro del directorio indicado por el path
 * 		filler - Este es un puntero a una función, la cual sabe como guardar una cadena dentro
 * 		         del campo buf
 */

static int callback_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
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

/*
 *      @PARAMETROS
 * 		path - El path es relativo al punto de montaje y es la forma mediante la cual debemos
 * 		       encontrar el archivo o directorio que nos solicitan
 * 		fi - es una estructura que contiene la metadata del archivo indicado en el path
 */



static int callback_open(const char *path, struct fuse_file_info *fi) {
    if (strcmp(path, DEFAULT_FILE_PATH) != 0)
        return -ENOENT;

    if ((fi->flags & 3) != O_RDONLY)
        return -EACCES;

    return 0;
}

/*      @PAPAMETROS
 *
 *      path - El path es relativo al punto de montaje y es la forma mediante la cual debemos
 * 		       encontrar el archivo o directorio que nos solicitan
 * 		buf - Este es el buffer donde se va a guardar el contenido solicitado
 * 		size - Nos indica cuanto tenemos que leer
 * 		offset - A partir de que posicion del archivo tenemos que leer
 *
 * */

static int callback_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
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

    return size;
}