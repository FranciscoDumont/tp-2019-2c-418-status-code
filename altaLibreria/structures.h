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

	//Header de ejemplos
	ABC,

	//Headers de SUSE
	SUSE_CREATE,
	SUSE_SCHEDULE_NEXT,
	SUSE_WAIT,
	SUSE_SIGNAL,
	SUSE_JOIN,
	SUSE_RETURN,

    //Headers de MUSE
	MUSE_INIT,
	MUSE_CLOSE,
	MUSE_ALLOC,
	MUSE_FREE,
	MUSE_GET,
	MUSE_CPY,
    MUSE_MAP,
    MUSE_SYNC,
    MUSE_UNMAP

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
