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
	RMDIR,//Borra un directorio
	OPEN, //Abre un archivo
	CLOSE,//Cierra el archivo
	READ, // Lee la informacion de un archivo abierto
	WRITE, // Escribe informacion en un archivo abierto
	READDIR, // Leer un directorio
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
