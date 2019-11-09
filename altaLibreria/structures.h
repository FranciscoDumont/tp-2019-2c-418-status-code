#ifndef ALTALIBRERIA_STRUCTURES_H_
#define ALTALIBRERIA_STRUCTURES_H_
#define IP_LENGTH 20

#include "connections.h"

typedef enum _NetworkDebugLevel {
	NW_NO_DISPLAY,
	NW_NETWORK_ERRORS,
	NW_ALL_DISPLAY
} NetworkDebugLevel;
NetworkDebugLevel NETWORK_DEBUG_LEVEL = NW_NO_DISPLAY;

typedef enum _MessageType {
	ABC,
	GETATTR, // Obtiene los atributos de un archivo
	READLINK, // Lee un simbolic link
	MKNOD, // Crea un archivo
	MKDIR, // Crea un directorio
	UNLINK, // Borrar un archivo
	SYMLINK, // Crea un simbolic link
	RENAME, // Cambia el nombre de un archivo
	LINK, // Crea un hard link a un archivo
	TRUNCATE, // Cambia el tamanio de un archivo
	OPEN, //Abre un archivo
	READ, // Lee la informacion de un archivo abierto
	WRITE, // Escribe informacion en un archivo abierto
	RELEASE, //INVESTIGAR SOBRE ESTO
	OPENDIR, // Abrir un directorio
	READDIR, // Leer un directorio
	CREATE, // Crea y abre un archivo
	UTIMENS, // Modifica el tiempo de modificacion de un archiv
	COPY_FILE_RANGE, // Copia un rango de datos de un archivo a otro
	SUSE_CREATE,
	SUSE_SCHEDULE_NEXT,
	SUSE_WAIT,
	SUSE_SIGNAL,
	SUSE_JOIN,
	SUSE_CLOSE
	//TODO: Agregar nuestros headers(tipos de mensajes a enviar)

} MessageType;

typedef struct _MessageHeader {
	MessageType type;
	int data_size;
} MessageHeader;


typedef struct {
	MessageHeader *header;
	void* stream;
} t_paquete;


#endif /* ALTALIBRERIA_STRUCTURES_H_ */
