//
// Created by utnso on 10/11/19.
//

#ifndef TOOLS_FUNCIONES_EMI_H
#define TOOLS_FUNCIONES_EMI_H

#include "tools/tools.h"
#include <string.h>
#include <stdio.h>
#include <commons/log.h>
#include <commons/bitarray.h>
#include <commons/collections/list.h>
#include "structs.h"

//GFile* hallar_padre(GBloque* bloque, char* nombrePadre, GBloque* disco);
t_list* hallar_posibles_nodos(char* nombreNodo);
int buscarPath(t_list* pathDividido);
#endif //TOOLS_FUNCIONES_EMI_H
