//
// Created by utnso on 23/10/19.
//

#ifndef TP_2019_2C_418_STATUS_CODE_OPERACIONES_H_
#define TP_2019_2C_418_STATUS_CODE_OPERACIONES_H_
#include <stddef.h>
#include <stdlib.h>
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <altaLibreria/connections.h>
#include <altaLibreria/structures.h>
//#include <fcntl.h>
//#include <unistd.h>

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
 * Esta Macro sirve para definir nuestros propios parametros que queremos que
 * FUSE interprete. Esta va a ser utilizada mas abajo para completar el campos
 * welcome_msg de la variable runtime_options
 */
#define CUSTOM_FUSE_OPT_KEY(t, p, v) { t, offsetof(struct t_runtime_options, p), v }

int sac_open(char* ruta );
int sac_mkdir(char* ruta);
int sac_mknode(char* ruta);
int sac_read(char* ruta );
int sac_write(char* ruta );
int sac_getattr(char* ruta );




#endif //TP_2019_2C_418_STATUS_CODE_OPERACIONES_H
