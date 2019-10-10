//
// Created by utnso on 01/10/19.
//

#ifndef SERVIDOR_PROTOCOLO_H
#define SERVIDOR_PROTOCOLO_H

#include "structures.h"

Paquete* crear_paquete(MessageType type);
void agregar_a_paquete(Paquete* paquete, void* valor, int tamanio);
int receive_header(int socket, Paquete* paquete);
int receive_data(int socket, Paquete* paquete, int data_size);
t_list* recibir_paquete(Paquete paquete);
void eliminar_paquete(Paquete* paquete);

#endif //SERVIDOR_PROTOCOLO_H