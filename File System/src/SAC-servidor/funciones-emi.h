//
// Created by utnso on 10/11/19.
//

#ifndef TOOLS_FUNCIONES_EMI_H
#define TOOLS_FUNCIONES_EMI_H

#include "tools/tools.h"
#include <string.h>
#include <stdio.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include "structs.h"
#include <time.h>

GFile* obtenerTablaNodos(GBloque* comienzoParticion);
GFile* hallar_padre(GBloque* bloque, char* nombrePadre, GBloque* disco);
#endif //TOOLS_FUNCIONES_EMI_H
