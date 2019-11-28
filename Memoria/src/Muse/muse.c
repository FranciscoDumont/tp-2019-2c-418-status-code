#include "muse.h"

int main() {
    logger = log_create("muse.log", "MUSE", 1, LOG_LEVEL_TRACE);
    read_memory_config();

    PROCESS_TABLE = list_create();

    CANTIDAD_PAGINAS_ACTUALES = 0;
	LIMITE_PAGINAS = config.memory_size / config.page_size;
	MAPA_MEMORIA_SIZE = ceil((double) LIMITE_PAGINAS / 8);
	char bitarray[MAPA_MEMORIA_SIZE];
	MAPA_MEMORIA = bitarray_create_with_mode(bitarray, sizeof(bitarray), LSB_FIRST);
	for(int indice = 0; indice < MAPA_MEMORIA_SIZE*8; indice++){
	    bitarray_clean_bit(MAPA_MEMORIA, indice);
	}

    MAIN_MEMORY = malloc(config.memory_size);

	log_info(logger, "Se pueden almacenar %d páginas", LIMITE_PAGINAS);

    pthread_t server_thread;
    pthread_create(&server_thread, NULL, server_function, NULL);
    tests_memoria();
    pthread_join(server_thread, NULL);


    free(MAPA_MEMORIA);
    free(MAIN_MEMORY);
    return 0;
}

void *server_function(void *arg) {

    int socket;

    if((socket = create_socket()) == -1) {
        printf("Error al crear el socket\n");
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
                    void *dst = *((void **) list_get(cosas, 0));
                    uint32_t src = *((uint32_t *) list_get(cosas, 1));
                    size_t n = *((size_t *) list_get(cosas, 2));
                    muse_get(dst, src, n);
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

        paginas_necesarias = (int) ceil((double)(tam + 2*sizeof(heap_metadata))/config.page_size);
        //TODO: verificar si hay paginas libres suficientes y usar un mutex?

        //En este caso hay frames disponibles para asignarle al segmento, probablemente se pueda sacar parte del codigo
        // de aca abajo porque se va a repetir
        if(cant_frames_libres() >= paginas_necesarias){
            segment_t* nuevo_segmento = crear_segmento(el_proceso, paginas_necesarias);

            log_info(logger, "Paginas necesarias: %d", paginas_necesarias);

            // Reservo los frames necesarios de la memoria principal
            asignar_paginas(paginas_necesarias, nuevo_segmento);

            // Tengo el segmento con la lista de paginas creada, ahora cargo los metadata en los lugares correspondientes de la MP
            asignar_primer_metadata(nuevo_segmento, tam);
            asignar_ultima_metadata(nuevo_segmento, tam, paginas_necesarias);

            list_add(el_proceso->segments, nuevo_segmento);

            ret_addr = nuevo_segmento->base + sizeof(heap_metadata);

        //TODO: Fran, averiguame esto please
        //En este caso no hay frames suficientes para asignar al nuevo segmento, deberia mandar paginas a MS para liberar
        // espacio o retornar un error o algo mas esoterico?
        } else {
            //TODO: liberar memoria para el nuevo segmento corriendo el algoritmo de reemplazo?
        }

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

        //Encontre un segmento con espacio libre suficiente
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
            mp_escribir_metadata(dir_fisica_primer_md, (espacio_libre - tam - sizeof(heap_metadata)), true);

            ret_addr = dir_virtual_md_libre + sizeof(heap_metadata);
        } else {

            //TODO verificar si hay algun segmento extendible(verificando si hay pag disponibles)
            //TODO crear nuevo segmento(verificando si hay pag disponibles)
        }
    }

    /**
     *
     * Hasta aca estamos bien, revisar todo_ lo demas
     *
    */
    //Analizaremos segmento a segmento de Heap,
    // verificando si alguno de los Headers de metadata se encuentra con el valor free para incorporar el malloc.
    //TODO: volar?
    /**
    paginas_necesarias = (int) ceil((double)(tam + 1*sizeof(heap_metadata))/config.page_size);
    int cantidad_segmentos_en_proceso = list_size(el_proceso->segments);
    // Por cada segmento busco una metadata libre
    for(int i=0; i<cantidad_segmentos_en_proceso; i++){
        segment_t* un_segmento = list_get(el_proceso->segments, i);
        page_t* primera_pagina = list_get(un_segmento->pages, 0);

        //Puntero al heap_metadata con espacio libre
        void* puntero = puntero_a_mp_del_primer_metadata_libre(un_segmento);
        if(puntero == NULL){continue;} // Si no hay un metadata libre pasa al sig segmento

        //Si hay espacio libre al final de un segmento
        if(((heap_metadata*)puntero)->size >= tam + 1*sizeof(heap_metadata)){
            int usado = segmento_ocupado_size(un_segmento);
            uint32_t dir_virtual = usado + un_segmento->base;
            // Si la metadata está libre y hay espacio: guardo ahí
            mp_escribir_metadata(puntero, tam, false); //sobreescrivo el actual metadata
            // Ahora tengogo que escribir el metadata que va a lo ultimo

            void* posicion_segundo_metadata = traducir_virtual(un_segmento, dir_virtual + tam);

            mp_escribir_metadata(posicion_segundo_metadata, (uint32_t)(un_segmento->size)-(usado+sizeof(heap_metadata)+tam), true);
            ret_addr = dir_virtual;
        }
    }
     */

    // En caso de no encontrarlo, buscaremos si hay algún segmento de Heap que se pueda extender.

    // Por último, en caso de no poder extender un segmento, deberá crear otro nuevo.

    return ret_addr;
}

void muse_free(uint32_t dir) {
}


int muse_get(void *dst, uint32_t src, size_t n) {
}

int muse_cpy(uint32_t dst, void *src, int n) {
}


uint32_t muse_map(char *path, size_t length, int flags) {
}


int muse_sync(uint32_t addr, size_t len) {
}


int muse_unmap(uint32_t dir) {
}

// Funciones Auxiliares

process_t* crear_proceso(int id){
    process_t* nuevo_proceso = malloc(sizeof(process_t));
    nuevo_proceso->pid = id;
    t_list* nueva_lista = list_create();
    nuevo_proceso->segments = nueva_lista;
    return nuevo_proceso;
}

segment_t* crear_segmento(proceso_t* process, int paginas_necesarias){
    int size = paginas_necesarias * config.page_size;
    int base = 0;
    if (list_size(el_proceso->segments) > 0) { // Si no es el primer segmento, veo cual será la base
        segment_t* last_segment = list_get(process->segments, list_size(process->segments) - 1);
        base = last_segment->base + last_segment->size;
    }
    segment_t* nuevo_segmento = malloc(sizeof(segment_t));
    nuevo_segmento->base = base;
    nuevo_segmento->size = size;
    nuevo_segmento->pages = list_create();
    return nuevo_segmento;
}

void asignar_paginas(int paginas_necesarias, segment_t* nuevo_segmento){
    for(int i = 0; i<paginas_necesarias; i++){
        int frame_libre = mp_buscar_frame_libre();
        bitarray_set_bit(MAPA_MEMORIA, frame_libre);
        page_t* nueva_pagina = crear_pagina(frame_libre, true, false, false);
        list_add(nuevo_segmento->pages, nueva_pagina);
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
    //TODO: verificar si esta bien el copiado de memoria y liberarlo
    heap_metadata* nueva_metadata = malloc(sizeof(heap_metadata));
    nueva_metadata->size = tam;
    nueva_metadata->isFree = isFree;
    const void * metadata = nueva_metadata;
    memcpy(espacio_libre, metadata, sizeof(heap_metadata));
    //free(nueva_metadata);
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

    while(tamanio < un_segmento->size){

        //Casteo la posicion del md a un md
        heap_metadata* md = (heap_metadata*)pos_md;

        //El md apunta a un espacio no vacio
        if(!una_metadata->isFree){

            tamanio += sizeof(heap_metadata) + md->size;
            //Actualizo a la sgte posicion del md
            pos_md = traducir_virtual(segmento, tamanio);

        //Encontre un md libre
        } else {

            //Hay espacio suficiente como para almacenar el nuevo bloque mas el md del final
            if(md->size > (tam + sizeof(heap_metadata))){

                //Hallo la direccion virtual que apunta al md que indica libre
                *puntero = tamanio + segmento->base;
                //Asigno la cant de espacio libre que poseia este cacho de memoria en bytes
                *espacio_libre = md->size;
                return true;

            //No hay espacio suficiente en el bloque libre
            } else {
                tamanio += sizeof(heap_metadata) + una_metadata->size;
                //Actualizo a la sgte posicion del md
                pos_md = traducir_virtual(segmento, tamanio);
            }
        }
    }
    return false;
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
