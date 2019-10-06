#ifndef SUSE_H
#define SUSE_H

#include <stdio.h>
#include <string.h>
#include <pthread.h>

#include <commons/config.h>
#include <commons/log.h>
#include <altaLibreria/connections.h>
#include <altaLibreria/structures.h>
#include "suseStructures.h"

t_config *leerConfig();
void* server_function(void* arg);

#endif //SUSE_H
