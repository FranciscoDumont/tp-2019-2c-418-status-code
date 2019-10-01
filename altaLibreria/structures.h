#ifndef DALIBRARY_STRUCTURES_H_
#define DALIBRARY_STRUCTURES_H_
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
	//TODO: Agregar nuestros headers

} MessageType;

typedef struct _MessageHeader {
	MessageType type;
	int data_size;
} MessageHeader;


typedef struct _Paquete
{
	MessageHeader header;
	void* stream;
} Paquete;


#endif /* DALIBRARY_STRUCTURES_H_ */
