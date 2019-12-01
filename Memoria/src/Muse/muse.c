#include "muse.h"

int main() {
    logger = log_create("muse.log", "MUSE", 1, LOG_LEVEL_TRACE);
    read_memory_config();

    PROCESS_TABLE = list_create();

    // Mapa Memoria
    CANTIDAD_PAGINAS_ACTUALES = 0;
	LIMITE_PAGINAS = config.memory_size / config.page_size;
	MAPA_MEMORIA_SIZE = ceil((double) LIMITE_PAGINAS / 8);
	char bitarray[MAPA_MEMORIA_SIZE];
	MAPA_MEMORIA = bitarray_create_with_mode(bitarray, sizeof(bitarray), LSB_FIRST);
	for(int indice = 0; indice < MAPA_MEMORIA_SIZE*8; indice++){
	    bitarray_clean_bit(MAPA_MEMORIA, indice);
	}
    log_info(logger, "Se pueden almacenar %d páginas", LIMITE_PAGINAS);

    // Mapa Swap
    LIMITE_PAGINAS_SWAP = config.swap_size / config.page_size;
    MAPA_SWAP_SIZE = ceil((double) LIMITE_PAGINAS_SWAP / 8);
    char bitarray_s[MAPA_SWAP_SIZE];
    MAPA_SWAP = bitarray_create_with_mode(bitarray, sizeof(bitarray_s), LSB_FIRST);
    for(int indice = 0; indice < MAPA_SWAP_SIZE*8; indice++){
        bitarray_clean_bit(MAPA_SWAP, indice);
    }
    SWAP = fopen("SWAP.bin", "w+b");
    /* Ejemplos
     * size_t fread(void *ptr, size_t size_of_elements, size_t number_of_elements, FILE *a_file);
     * size_t fwrite(const void *ptr, size_t size_of_elements, size_t number_of_elements, FILE *a_file)
     */

    // Estructuras para el reemplazo de paginas
    FRAMES_PAGINAS = list_create();
    log_info(logger, "Se pueden almacenar %d páginas en el area de SWAP", LIMITE_PAGINAS_SWAP);

    MAIN_MEMORY = malloc(config.memory_size);

    pthread_t server_thread;
    pthread_create(&server_thread, NULL, server_function, NULL);
    tests_memoria();
    pthread_join(server_thread, NULL);


    free(MAPA_MEMORIA);
    free(MAIN_MEMORY);
    free(SWAP_PAGINAS);
    return 0;
}

void *server_function(void *arg) {

    int socket;

    if((socket = create_socket()) == -1) {
        log_error(logger, "Error al crear el socket");
    	//TODO:retornar algun error?
    }
    if ((bind_socket(socket, config.listen_port)) == -1) {
        log_error(logger, "Error al bindear el socket");
        //TODO:retornar algun error?
    }

    //TODO revisar si esta bien
    //--Funcion que se ejecuta cuando se conecta un nuevo programa
    void new(int fd, char *ip, int port) {
        if(&fd != null && ip != null && &port != null) {
            log_info(logger, "Nueva conexión");
        }
    }

    //TODO revisar si esta bien
    //--Funcion que se ejecuta cuando se pierde la conexion con un cliente
    void lost(int fd, char *ip, int port) {
        if(&fd == null && ip == null && &port == null){
            log_info(logger, "Se perdió una conexión");
            //Cierro la conexión fallida
            log_info(logger, "Cerrando conexión");
            close(fd);
        }
    }

    //--funcion que se ejecuta cuando se recibe un nuevo mensaje de un cliente ya conectado
    void incoming(int fd, char *ip, int port, MessageHeader *headerStruct) {

        t_list *cosas = receive_package(fd, headerStruct);

        //TODO:limpiar el case
        switch (headerStruct->type) {
            case MUSE_INIT:;
                {
                    // Ejecuto muse_init
                    int resultado = muse_init(fd, ip, port);
                    // Le respondo a libMuse
                    t_paquete *package = create_package(MUSE_INIT);
                    void *respuesta = malloc(sizeof(int));
                    *((int *) respuesta) = resultado;
                    add_to_package(package, respuesta, sizeof(int));
                    send_package(package, fd);
                    break;
                }

            case MUSE_CLOSE:;
                {
                    muse_close();
                    break;
                }

            case MUSE_ALLOC:;
                {
                    // Guardo el valor que recibo en una variable
                    uint32_t tam = *((uint32_t *) list_get(cosas, 0));
                    // Ejecuto el malloc
                    uint32_t resultado = muse_alloc(tam, fd);
                    // Le respondo a libMuse
                    t_paquete *package = create_package(MUSE_INIT);
                    void *respuesta = malloc(sizeof(int));
                    *((int *) respuesta) = resultado;
                    add_to_package(package, respuesta, sizeof(int));
                    send_package(package, fd);
                    break;
                }

            case MUSE_FREE:;
                {
                    uint32_t dir = *((uint32_t *) list_get(cosas, 0));
                    muse_free(dir);
                    break;
                }

            case MUSE_GET:;
                {
                    uint32_t src = *((uint32_t *) list_get(cosas, 0));
                    size_t n = *((size_t *) list_get(cosas, 1));

                    void* resultado = muse_get(src, n, fd);

                    // Le respondo a libMuse
                    t_paquete *package = create_package(MUSE_GET);
                    void* respuesta = malloc(n);
                    respuesta = resultado;
                    add_to_package(package, respuesta, n);
                    send_package(package, fd);
                    break;
                }

            case MUSE_CPY:;
                {
                    uint32_t dst = *((uint32_t *) list_get(cosas, 0));
                    void *src = *((void **) list_get(cosas, 1));
                    int n = *((int *) list_get(cosas, 2));
                    muse_cpy(dst, src, n);
                    break;
                }

            case MUSE_MAP:;
                {
                    char *path = ((char *) list_get(cosas, 0));
                    size_t length = *((size_t*) list_get(cosas, 1));
                    int flags = *((int*) list_get(cosas, 2));
                    muse_map(path,length,flags);
                    break;
                }

            case MUSE_SYNC:;
                {
                    uint32_t addr = *((uint32_t *) list_get(cosas, 0));
                    size_t len = *((size_t *) list_get(cosas, 1));
                    muse_sync(addr,len);
                    break;
                }

            case MUSE_UNMAP:;
                {
                    uint32_t dir = *((uint32_t *) list_get(cosas, 0));
                    muse_unmap(dir);
                    break;
                }

            default: {
                log_warning(logger, "Operacion desconocida. No quieras meter la pata\n");
                break;
            }
        }
    }
    log_info(logger, "Hilo de servidor iniciado...");
    start_server(socket, &new, &lost, &incoming);
}

void read_memory_config() {
    config_file = config_create("config");

    if (!config_file) {
        log_error(logger, "No se encontró el archivo de configuración");
        return;
    }

    config.listen_port = config_get_int_value(config_file, "LISTEN_PORT");
    config.memory_size = config_get_int_value(config_file, "MEMORY_SIZE");
    config.page_size = config_get_int_value(config_file, "PAGE_SIZE");
    config.swap_size = config_get_int_value(config_file, "SWAP_SIZE");

    log_info(logger, \
        "Configuración levantada\n\tLISTEN_PORT: %d\n\tMEMORY_SIZE: %d\n\tPAGE_SIZE: %d\n\tSWAP_SIZE: %d.", \
        config.listen_port, \
        config.memory_size, \
        config.page_size, \
        config.swap_size);
}

/*
  ██████╗ ██████╗ ██████╗ ███████╗
 ██╔════╝██╔═══██╗██╔══██╗██╔════╝
 ██║     ██║   ██║██████╔╝█████╗
 ██║     ██║   ██║██╔══██╗██╔══╝
 ╚██████╗╚██████╔╝██║  ██║███████╗
  ╚═════╝ ╚═════╝ ╚═╝  ╚═╝╚══════╝
*/

int muse_init(int id, char *ip, int puerto) {
    log_info(logger, "Empieza el muse_init");

    // Loggeo el valor del id que recibi
    log_info(logger, "El fd que recibo de libMuse es: %d", id);
    log_info(logger, "El ip que recibo de libMuse es: %s", ip);
    log_info(logger, "El puerto que recibo de libMuse es: %d", puerto);

    process_t* nuevo_proceso = crear_proceso(id);
    list_add(PROCESS_TABLE, nuevo_proceso);
    log_info(logger, "Se creo un nuevo proceso");

    return 1;
}

void muse_close() {
}

uint32_t muse_alloc(uint32_t tam, int id_proceso) {
    // Veo cuantas paginas necesito
    int paginas_necesarias;
    process_t* el_proceso = buscar_proceso(id_proceso);
    t_list* segments = el_proceso->segments;
    uint32_t ret_addr;

    //En este caso el proceso no tiene ningun segmento
    if(list_size(segments) == 0){

        ret_addr = nuevo_segmento(tam, el_proceso);

    //El proceso ya poseia segmentos
    } else {

        //Verifico si hay algun segmento con capacidad de almacenaje
        paginas_necesarias = (int) ceil((double)(tam + 1*sizeof(heap_metadata))/config.page_size);
        //Aca deberia almacenar la direccion virtual que apunta al md que indica el espacio libre
        uint32_t dir_virtual_md_libre = 0;
        uint32_t espacio_libre = 0;
        bool tiene_espacio(void* _segmento){
            segment_t* segmento = (segment_t*)_segmento;
            return tiene_espacio_libre(segmento, &dir_virtual_md_libre, tam, &espacio_libre);
        }
        segment_t* segmento_a_ocupar = (segment_t*)list_find(el_proceso->segments, tiene_espacio);

        //Encontre un segmento con espacio libre suficiente para almacenar el nuevo cacho
        if(segmento_a_ocupar != null){

            //Hallo la direccion fisica del pimer md para corregirlo
            void* dir_fisica_primer_md = traducir_virtual(segmento_a_ocupar, dir_virtual_md_libre);

            //Sobreescribo la md que me mostraba el espacio libre
            mp_escribir_metadata(dir_fisica_primer_md, tam, false);

            //Hallo la direccion fisica del segundo md la cual se halla sumandole a la dir virtual del primer md, el
            // tamaño de un md y el tamaño de lo reservado, y traduciendolo
            void* dir_fisica_segundo_md = traducir_virtual(segmento_a_ocupar, dir_virtual_md_libre + sizeof(heap_metadata) + tam);

            //Escribo la md a final del bloque reservado, el espacio nuevo sera el espacio viejo - el tamaño reservado
            // - el tamaño de un md
            mp_escribir_metadata(dir_fisica_segundo_md, (espacio_libre - tam - sizeof(heap_metadata)), true);

            ret_addr = dir_virtual_md_libre + sizeof(heap_metadata);

        //No hay espacio para almacenar en un segmento directamente
        } else {

            //Si hay un solo segmento no tengo que revisar demasiado, reduzco overhead
            if(list_size(segments) == 1){

                //Si hay un solo segmento y no es de tipo mmap, SIEMPRE va a ser extendible
                segment_t* segmento = list_get(segments, 0);
                if(!segmento->is_map){

                    ret_addr = extender_segmento(tam, segmento);

                //En este caso el segmento es mmap, tengo que crear uno nuevo
                } else {

                    ret_addr = nuevo_segmento(tam, el_proceso);
                }

            //En este caso hay mas de 1 segmento, busco el primero cuya direccion virtual no pise la del siguiente si
            // le agrego las paginas necesarias
            } else {

                segment_t* segmento_a_extender = buscar_segmento_a_extender(tam, el_proceso);

                //Existe un segmento extendible
                if(segmento_a_extender != null){

                    ret_addr = extender_segmento(tam, segmento_a_extender);
                //No existe un segmento extendible
                } else {

                    ret_addr = nuevo_segmento(tam, el_proceso);
                }
            }
        }
    }

    return ret_addr;
}

void muse_free(uint32_t dir) {
}

void* muse_get(uint32_t direccion, size_t tam, int id_proceso){
    process_t* el_proceso = buscar_proceso(id_proceso);
    segment_t* el_segmento = buscar_segmento_por_direccion(el_proceso, direccion);


}

int muse_cpy(uint32_t dst, void *src, int n) {
}

uint32_t muse_map(char *path, size_t length, int flags) {
}

int muse_sync(uint32_t addr, size_t len) {
}

int muse_unmap(uint32_t dir) {
}

/*
 █████╗ ██╗   ██╗██╗  ██╗
██╔══██╗██║   ██║╚██╗██╔╝
███████║██║   ██║ ╚███╔╝
██╔══██║██║   ██║ ██╔██╗
██║  ██║╚██████╔╝██╔╝ ██╗
╚═╝  ╚═╝ ╚═════╝ ╚═╝  ╚═╝
*/

process_t* crear_proceso(int id){
    process_t* nuevo_proceso = malloc(sizeof(process_t));
    nuevo_proceso->pid = id;
    t_list* nueva_lista = list_create();
    nuevo_proceso->segments = nueva_lista;
    return nuevo_proceso;
}

segment_t* crear_segmento(proceso_t* process, int paginas_necesarias, bool is_map){
    int size = paginas_necesarias * config.page_size;
    int base = 0;
    if (list_size(el_proceso->segments) > 0) { // Si no es el primer segmento, veo cual será la base
        segment_t* last_segment = list_get(process->segments, list_size(process->segments) - 1);
        base = last_segment->base + last_segment->size;
    }
    segment_t* nuevo_segmento = malloc(sizeof(segment_t));
    nuevo_segmento->base = base;
    nuevo_segmento->size = size;
    nuevo_segmento->is_map = is_map;
    nuevo_segmento->pages = list_create();
    return nuevo_segmento;
}

void asignar_paginas(int paginas_necesarias, segment_t* nuevo_segmento){
    for(int i = 0; i<paginas_necesarias; i++){
        int frame_libre = mp_buscar_frame_libre();
        bitarray_set_bit(MAPA_MEMORIA, frame_libre);
        page_t* nueva_pagina = crear_pagina(frame_libre, true, false, true);
        list_add(nuevo_segmento->pages, nueva_pagina);
        list_add(FRAMES_PAGINAS, nueva_pagina); // Agrego la pagina a la lista de frames
    }
}

page_t* crear_pagina(int frame, int presence_bit, int modified_bit, int use_bit){
    page_t* nueva_pagina = malloc(sizeof(page_t));
    nueva_pagina->frame = frame;
    nueva_pagina->presence_bit = presence_bit;
    nueva_pagina->modified_bit = modified_bit;
    nueva_pagina->use_bit = use_bit;
    return nueva_pagina;
}

//Me parece que me excedi con estas dos funciones, no se si se van a volver a utilizar
void asignar_primer_metadata(segment_t* segment, int tam){
    page_t* primera_pagina = list_get(segment->pages, 0);
    void* posicion_primer_metadata = MAIN_MEMORY + config.page_size * primera_pagina->frame;
    mp_escribir_metadata(posicion_primer_metadata, tam, false);
}

void asignar_ultima_metadata(segment_t* segment, int tam, int paginas_necesarias){
    void* posicion_segundo_metadata = traducir_virtual(segment, tam + sizeof(heap_metadata));
    mp_escribir_metadata(posicion_segundo_metadata, (uint32_t)(paginas_necesarias * config.page_size) - tam - 2 * sizeof(heap_metadata), true);
}

void* mp_escribir_metadata(void* espacio_libre, uint32_t tam, int isFree){
    heap_metadata* nueva_metadata = malloc(sizeof(heap_metadata));
    nueva_metadata->size = tam;
    nueva_metadata->isFree = isFree;
    const void * metadata = nueva_metadata;
    memcpy(espacio_libre, metadata, sizeof(heap_metadata));
    free(nueva_metadata);
    return espacio_libre + sizeof(heap_metadata);
}

int mp_buscar_frame_libre(){
	int i;
	int nro_frame = -1; // Si no encuentra un frame libre va a devolver -1
	for (i = 0; i<MAPA_MEMORIA_SIZE; ++i){
	    if (! bitarray_test_bit(MAPA_MEMORIA, i)){
	        nro_frame = i;
            break; //si el lugar está libre salgo del for
	    }
	}
    return nro_frame;
}

int ms_buscar_frame_libre(){
    int i;
    int nro_frame = -1; // Si no encuentra un frame libre va a devolver -1
    for (i = 0; i<MAPA_SWAP_SIZE; ++i){
        if (! bitarray_test_bit(MAPA_SWAP, i)){
            nro_frame = i;
            break; //si el lugar está libre salgo del for
        }
    }
    return nro_frame;
}

int cant_frames_libres(){
    int i;
    int cant_frames = 0;
    for (i = 0; i<MAPA_MEMORIA_SIZE; ++i){
        if (! bitarray_test_bit(MAPA_MEMORIA, i)){
            cant_frames++;
        }
    }
    return cant_frames;
}

process_t* buscar_proceso(int id_proceso){
    bool key_search(void * un_proceso){
        process_t * process = (process_t *) un_proceso;
        return process->pid == id_proceso;
    }
    return (process_t*)list_find(PROCESS_TABLE, key_search);
}

char* mapa_memoria_to_string(){
	char* resultado = string_new();
	string_append(&resultado, string_itoa(MAPA_MEMORIA_SIZE*8));
	string_append(&resultado, " [");
	int i;
	for(i=0; i<(MAPA_MEMORIA->size*8); i++){
		string_append(&resultado, string_itoa(bitarray_test_bit(MAPA_MEMORIA, i)));
		string_append(&resultado, "|");
	}
	string_append(&resultado, "]");
	return resultado;
}

bool tiene_espacio_libre(segment_t* segmento, uint32_t* puntero, uint32_t tam, uint32_t espacio_libre){

    //Hallo la direccion inicial del segmento, la cual va a apuntar al primer md
    void* pos_md = traducir_virtual(segmento, 0);
    int tamanio = 0;

    while(tamanio < segmento->size){

        //Casteo la posicion del md a un md
        heap_metadata* md = (heap_metadata*)pos_md;

        //El md apunta a un espacio no vacio
        if(!md->isFree){

            tamanio += sizeof(heap_metadata) + md->size;
            //Actualizo a la sgte posicion del md
            pos_md = traducir_virtual(segmento, tamanio);

        //Encontre un md libre
        } else {

            //Hay espacio suficiente como para almacenar el nuevo bloque mas el md del final
            if(md->size >= (tam + sizeof(heap_metadata))){

                //Hallo la direccion virtual que apunta al md que indica libre
                *puntero = tamanio + segmento->base;
                //Asigno la cant de espacio libre que poseia este cacho de memoria en bytes
                *espacio_libre = md->size;
                return true;

            //No hay espacio suficiente en el bloque libre
            } else {
                tamanio += sizeof(heap_metadata) + md->size;
                //Actualizo a la sgte posicion del md
                pos_md = traducir_virtual(segmento, tamanio);
            }
        }
    }
    return false;
}

uint32_t buscar_ultimo_md(segment_t* segmento){

    //Hallo la direccion inicial del segmento, la cual va a apuntar al primer md
    void* pos_md = traducir_virtual(segmento, 0);
    int tamanio = 0;

    while(tamanio < segmento->size){

        //Casteo la posicion del md a un md
        heap_metadata* md = (heap_metadata*)pos_md;

        //El md apunta a un espacio no vacio
        if(!md->isFree){

            tamanio += sizeof(heap_metadata) + md->size;
            //Actualizo a la sgte posicion del md
            pos_md = traducir_virtual(segmento, tamanio);

        //Encontre un md libre
        } else {

            tamanio += sizeof(heap_metadata) + md->size;

            //LLegue al final del segmento
            if(tamanio == segmento->size){

                //Retorno la direccion virtual del ultimo md
                return segmento->base + tamanio - (sizeof(heap_metadata) + md->size);
            } else {

                //Actualizo a la sgte posicion del md
                pos_md = traducir_virtual(segmento, tamanio);
            }
        }
    }
}

//TODO: completar el caso en que no hay frames disponibles
uint32_t nuevo_segmento(int tam, process_t* proceso){

    int paginas_necesarias = (int) ceil((double)(tam + 2*sizeof(heap_metadata))/config.page_size);
    uint32_t ret_addr = 0;
    int frames_libres = cant_frames_libres();

    //En este caso hay frames disponibles para asignarle al segmento, probablemente se pueda sacar parte del codigo
    // de aca abajo porque se va a repetir
    if(frames_libres >= paginas_necesarias){

        //Creo un segmento nuevo
        segment_t* segmento = crear_segmento(proceso, paginas_necesarias, false);

        log_info(logger, "Paginas necesarias: %d", paginas_necesarias);

        // Reservo los frames necesarios de la memoria principal
        asignar_paginas(paginas_necesarias, segmento);

        // Tengo el segmento con la lista de paginas creada, ahora cargo los metadata en los lugares correspondientes
        // de la MP
        asignar_primer_metadata(segmento, tam);
        asignar_ultima_metadata(segmento, tam, paginas_necesarias);

        list_add(proceso->segments, segmento);

        ret_addr = segmento->base + sizeof(heap_metadata);

        //En este caso no hay frames suficientes para asignar al nuevo segmento, deberia mandar paginas a MS para liberar
        // espacio
    } else {

        //Liberar frames
        int error = 0;
        for(int i = 0; i < (paginas_necesarias - frames_libres); i++){
            if(algoritmo_de_reemplazo() == -1){
                error = -1;
                break;
            }
        }
        if(error == -1){
            ret_addr = NULL;
        } else {
            ret_addr = nuevo_segmento(tam, segmento);
        }
    }

    return ret_addr;
}

//TODO: completar el caso en que no hay frames disponibles
uint32_t extender_segmento(int tam, segment_t* segmento){

    int paginas_necesarias = (int) ceil((double)(tam + 1*sizeof(heap_metadata))/config.page_size);
    uint32_t ret_addr = 0;
    int frames_libres = cant_frames_libres();

    //Si hay frames libres, el segmento es extendible directamente
    if(frames_libres >= paginas_necesarias){

        //Agrego las paginas necesarias
        asignar_paginas(paginas_necesarias, segmento);
        //Actualizo tamaño del segmento
        segmento->size += paginas_necesarias * config.page_size;
        //Buscar el ultimo md
        uint32_t dir_ultimo_md = buscar_ultimo_md(segmento);
        //Hallo la direccion fisica del md para actualizarlo
        void* dir_fisica_ultimo_md = traducir_virtual(segmento, dir_ultimo_md);
        //Actualizo al md
        mp_escribir_metadata(dir_fisica_ultimo_md, tam, false);
        //Hallo la direccion fisica a ocupar por el nuevo ultimo md
        void* dir_fisica_nuevo_ultimo_md = traducir_virtual(segmento, dir_ultimo_md + sizeof(heap_metadata) + tam);
        //Escribir el nuevo ultimo md
        mp_escribir_metadata(dir_fisica_nuevo_ultimo_md, tam, false);

        ret_addr = dir_ultimo_md + sizeof(heap_metadata);
    //El segmento no es extendible directamente
    } else {

        //Liberar frames
        int error = 0;
        for(int i = 0; i < (paginas_necesarias - frames_libres); i++){
            if(algoritmo_de_reemplazo() == -1){
                error = -1;
                break;
            }
        }
        if(error == -1){
            ret_addr = NULL;
        } else {
            ret_addr = extender_segmento(tam, segmento);
        }
    }

    return ret_addr;
}

segment_t* buscar_segmento_a_extender(int tam, process_t* proceso){

    int iter = 0;

    while(1){

        //Posibilidades:
            //-primero map y segundo null -> return null
            //-primero map y segundo map -> continue
            //-primero map y segundo no map -> continue
            //-primero no map y segundo map -> analizar dir virtual
            //-primero no map y segundo no map -> analizar dir virtual
            //-primero no map y segundo null -> return primero

        segment_t* primero = list_get(proceso->segments, iter);
        segment_t* segundo = list_get(proceso->segments, iter + 1);
        iter++;

        if(primero->is_map && segundo == null){

            return null;
        } else if(primero->is_map && segundo->is_map){

            continue;
        } else if(primero->is_map && !segundo->is_map) {

            continue;
        } else if(!primero->is_map && segundo->is_map){

            //Hallo la direccion virtual del ultimo md del primer segmento
            uint32_t dir_md = buscar_ultimo_md(primero);
            //Hallo la base del segundo
            uint32_t base_segundo = (uint32_t)segundo->base;

            //Si la direccion del ultimo md del primero mas el tamaño a alocar es menor a la base del segundo, retorno
            // el primero sino continuo
            if((dir_md + tam + sizeof(heap_metadata)) < base_segundo){

                return primero;
            } else {

                continue;
            }
        } else if(!primero->is_map && !segundo->is_map){

            //Hallo la direccion virtual del ultimo md del primer segmento
            uint32_t dir_md = buscar_ultimo_md(primero);
            //Hallo la base del segundo
            uint32_t base_segundo = (uint32_t)segundo->base;

            //Si la direccion del ultimo md del primero mas el tamaño a alocar es menor a la base del segundo, retorno
            // el primero sino continuo
            if((dir_md + tam + sizeof(heap_metadata)) < base_segundo){

                return primero;
            } else {

                continue;
            }
        } else {

            return primero;
        }

    }
}

//TODO: Ver si esta funcion podria volar
void* puntero_a_mp_del_primer_metadata_libre(segment_t* un_segmento){
    int tamanio_ocupado = segmento_ocupado_size(un_segmento);
    if(tamanio_ocupado == un_segmento->size){
        return NULL;
    }
    return traducir_virtual(un_segmento, tamanio_ocupado - sizeof(heap_metadata));
}

void* traducir_virtual(segment_t* un_segmento, uint32_t direccion_virtual){
    int numero_pagina = direccion_virtual / config.page_size;
    int offset = direccion_virtual % config.page_size;
    page_t* la_pagina = list_get(un_segmento->pages, numero_pagina);
    return MAIN_MEMORY + (config.page_size * la_pagina->frame) + offset;
}

//TODO: Ver si esta funcion podria volar
int segmento_ocupado_size(segment_t* un_segmento){
    //Esto tendria que reventar, estamos copiando cosas arriba de una memoria no reservada
    heap_metadata* una_metadata;
    void* puntero = traducir_virtual(un_segmento, 0);
    int tamanio = 0;

    while(tamanio < un_segmento->size){
        memcpy(una_metadata, puntero, sizeof(heap_metadata));
        //TODO: arreglar esto, si tengo un cacho de memoria libre en el medio del segmento esto va a cortar, pero puede llegar a haber espacio ocupado luego del mismo
        if(!una_metadata->isFree){
            tamanio += sizeof(heap_metadata) + una_metadata->size;
            puntero = traducir_virtual(un_segmento, tamanio);
        }else { // Encontre un metadata Free
            tamanio += sizeof(heap_metadata);
            break;
        }
    }
    return tamanio;
}

segment_t* buscar_segmento_por_direccion(process_t* el_proceso, uint32_t direccion){
    bool search(void * un_segmento) {
        segment_t* segmento = (segment_t *) un_segmento;
        uint32_t piso = segmento->base;
        uint32_t techo = piso + segmento->size
        return piso <= direccion && direccion <= techo;
    }
    return (segment_t*) list_find(el_proceso->segments, search);
}

int algoritmo_de_reemplazo(){
/*
    Clock modificado:
    1. Empezando desde la posición actual del puntero, recorrer la lista de marcos.
    Durante el recorrido, dejar el bit de uso (U) intacto.
    El primer marco que se encuentre con U = 0 y M = 0 se elige para el reemplazo.
*/
    page_t* pagina_a_reemplazar;
    int frames_paginas_size = list_size(FRAMES_PAGINAS);

    void paso_1(){
        for(;PUNTERO_REEMPLAZO<frames_paginas_size;PUNTERO_REEMPLAZO++){
            page_t* pagina_tmp = list_get(FRAMES_PAGINAS, PUNTERO_REEMPLAZO);
            bool uso = pagina_tmp->use_bit;
            bool modificado = pagina_tmp->modified_bit;
            if (!uso && !modificado){
                pagina_a_reemplazar = pagina_tmp;
                return;
            }
        }
        // Si llega aca es porque recorrio todos los frames
        PUNTERO_REEMPLAZO = 0; // Vuelvo el puntero a 0
        return;
    }
/*
    2. Si el paso 1 falla, recorrer nuevamente, buscando un marco con U = 0 y M = 1.
    El primer marco que cumpla la condición es seleccionado para el reemplazo.
    Durante este recorrido, cambiar el bit de uso a 0 de todos los marcos que no se elijan.
*/

 /*
    3. Si el paso 2 falla, volver al paso 1.
 */
    //Busco algun frame libre en la ms
    int frame_ms = ms_buscar_frame_libre();

    //Si el frame es -1 significa que no habia ningun frame disponible en la ms
    if(frame_ms == -1){
        return frame_ms;
    }


    //Encontre algun frame disponible en la ms
    //TODO: Hacer list/array de estructuras que relacionen el nmro de frame con la pagina que tiene asignada


    /* TODO: Quiza borrar esto luego
    int i;
    int nro_frame = -1; // Si no encuentra un frame libre va a devolver -1
    for (i = 0; i<MAPA_MEMORIA_SIZE; ++i){
        if (! bitarray_test_bit(MAPA_MEMORIA, i)){
            nro_frame = i;
            break; //si el lugar está libre salgo del for
        }
    }
    */
    return 0;
}

/*
████████╗███████╗███████╗████████╗███████╗
╚══██╔══╝██╔════╝██╔════╝╚══██╔══╝██╔════╝
   ██║   █████╗  ███████╗   ██║   ███████╗
   ██║   ██╔══╝  ╚════██║   ██║   ╚════██║
   ██║   ███████╗███████║   ██║   ███████║
   ╚═╝   ╚══════╝╚══════╝   ╚═╝   ╚══════╝
*/

void tests_memoria(){
    //mem_assert recive mensaje de error y una condicion, si falla el test lo loggea
    #define mem_assert(message, test) do { if (!(test)) { log_error(test_logger, message); tests_fail++; } tests_run++; } while (0)
    t_log * test_logger = log_create("memory_tests.log", "MEM", true, LOG_LEVEL_TRACE);
    int tests_run = 0;
    int tests_fail = 0;

    int id = 1;
    muse_init(id, "localhost", 5003);

    int tmp;
    tmp = muse_alloc(10, id);
    mem_assert("Alloc 1", tmp == 5);

    tmp = muse_alloc(16, id);
    mem_assert("Alloc 2", tmp == 5+10+5);

    tmp = muse_alloc(150, id);
    mem_assert("Alloc 3", tmp == 5+10+5+16+5);

    tmp = muse_alloc(88, id);
    mem_assert("Alloc 4", tmp == 5+10+5+16+5+150+5);

    log_warning(test_logger, "Pasaron %d de %d tests", tests_run-tests_fail, tests_run);
    log_destroy(test_logger);
}