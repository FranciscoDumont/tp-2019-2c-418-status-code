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
	ABC
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


typedef struct _t_new_comm{
    int fd;
    char* ip;
    int port;
    t_list* received;
} t_new_comm;


#endif /* ALTALIBRERIA_STRUCTURES_H_ */
