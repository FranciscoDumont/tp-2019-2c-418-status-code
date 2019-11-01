#include "muse.h"

int main() {
    logger = log_create("muse.log", "MUSE", 1, LOG_LEVEL_TRACE);
    read_memory_config();

    PROCESS_TABLE = list_create();

    CANTIDAD_PAGINAS_ACTUALES = 0;
	LIMITE_PAGINAS = config.memory_size / config.page_size;
	MAPA_MEMORIA_SIZE = LIMITE_PAGINAS;
	MAPA_MEMORIA = calloc(LIMITE_PAGINAS, sizeof(int));

    MAIN_MEMORY = malloc(config.memory_size);

	log_info(logger, "Se pueden almacenar %d páginas", LIMITE_PAGINAS);


    pthread_t server_thread;
    pthread_create(&server_thread, NULL, server_function, NULL);
    pthread_join(server_thread, NULL);


    free(MAPA_MEMORIA);
    free(MAIN_MEMORY);
    return 0;
}

int validate_create_sockets(int socket) {
    if(socket == -1) {
        printf("Error al crear el socket\n");
        return -1;
    }
}

int validate_bind_socket(int socket, int port){
    if ((bind_socket(socket, port)) == -1) {
        log_error(logger, "Error al bindear el socket");
        return -2;
    }
}

void *server_function(void *arg) {

    int port = config.listen_port;
    int socket = create_socket();
    validate_create_sockets(socket);
    validate_bind_socket(socket, port);


    //TODO revisar si esta bien
    //--Funcion que se ejecuta cuando se conecta un nuevo programa
    void new(int fd, char *ip, int port) {
        if(fd != null && *ip != null && port != null) {
            log_info(logger, "Nueva conexión");
        }
    }

    //TODO revisar si esta bien
    //--Funcion que se ejecuta cuando se pierde la conexion con un cliente
    void lost(int fd, char *ip, int port) {
        if(fd == null || *ip == null || port == null){
            log_info(logger, "Se perdió una conexión");
        }
    }

    //--funcion que se ejecuta cuando se recibe un nuevo mensaje de un cliente ya conectado
    void incoming(int fd, char *ip, int port, MessageHeader *headerStruct) {

        t_list *cosas = receive_package(fd, headerStruct);

        switch (headerStruct->type) {
            case MUSE_INIT:;
                {
                    // Guardo el entero que recibo en una variable
                    int id = *((int *) list_get(cosas, 0));

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

    process_t* nuevo_proceso = crear_proceso(id);
    list_add(PROCESS_TABLE, nuevo_proceso);
    log_info(logger, "Se creo un nuevo proceso");

    return 1;
}

//todo implementar funciones
void muse_close() {
}


uint32_t muse_alloc(uint32_t tam, int id_proceso) {
    // Veo cuantas paginas necesito
    int paginas_necesarias = (int) ceil((double) tam/config.page_size);
    log_info(logger, "Paginas necesarias: %d", paginas_necesarias);

    process_t* el_proceso = buscar_proceso(id_proceso);

    // Si el proceso ya tiene segmentos veo de asignarlo ahi
    //  xd
    // Si no tiene segmentos creo uno nuevo
    void* espacio_libre = mp_buscar_espacio_libre(paginas_necesarias);
    segment_t* nuevo_segmento = crear_segmento(espacio_libre, false);
    list_add(el_proceso->segments, nuevo_segmento);
    mp_escribir_metadata(espacio_libre, tam, false);
    mp_escribir_metadata(espacio_libre + tam, (uint32_t)paginas_necesarias-tam, true);

    return -1;
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


segment_t* crear_segmento(void* memory_pointer, int is_shared){
    segment_t* nuevo_segmento = malloc(sizeof(segment_t));
    nuevo_segmento->is_shared = is_shared;
    nuevo_segmento->memory_pointer = memory_pointer;
    t_list* nueva_lista = list_create();
    nuevo_segmento->pages = nueva_lista;
    return nuevo_segmento;
}


page_t* crear_pagina(int presence_bit, int modified_bit){
    page_t* nueva_pagina = malloc(sizeof(page_t));
    nueva_pagina->presence_bit = presence_bit;
    nueva_pagina->modified_bit = modified_bit;
    return nueva_pagina;
}


void* mp_escribir_metadata(void* espacio_libre, uint32_t tam, int esta_libre){
    heap_metadata* nueva_metadata = malloc(sizeof(heap_metadata));
    nueva_metadata->size = tam;
    nueva_metadata->isFree = esta_libre;
    memcpy(espacio_libre, nueva_metadata, sizeof(heap_metadata));
    return espacio_libre + sizeof(heap_metadata);
}


void* mp_buscar_espacio_libre(int paginas_necesarias){
	// esta funcion no setea los espacios como ocupados cuando los encuentra
	// asi que debera hacer eso cada vez que es llamada
	int i;
	bool esta_todo_ok = false;
	for (i = 0; i<MAPA_MEMORIA_SIZE; ++i){
	    if (MAPA_MEMORIA[i] != 0){
            continue; //si el lugar está ocupado paso al siguiente
	    }
        int ii = 0;
        int sumatoria = 0;
        // me fijo si los proximos n lugares estan vacios
        while (ii < paginas_necesarias){
            sumatoria += MAPA_MEMORIA[i];
            ii++;
        }
        if (sumatoria == 0){
            esta_todo_ok = true;
            break; //salgo del for
        }
	}

	void* resultado;
	if (esta_todo_ok){
        log_info(logger, "Mapa memoria consigue el indice: %d", i);
        resultado = MAIN_MEMORY + (i * config.page_size);
    }else {
        log_warning(logger, "No se pudo encontrar el espacio solicitado");
        resultado = null;
    }
    return resultado;
}


char* mapa_memoria_to_string(){
	char* resultado = string_new();
	string_append(&resultado, string_itoa(MAPA_MEMORIA_SIZE));
	string_append(&resultado, " [");
	int i;
	for(i=0; i<MAPA_MEMORIA_SIZE; i++){
		string_append(&resultado, string_itoa(MAPA_MEMORIA[i]));
		string_append(&resultado, "|");
	}
	string_append(&resultado, "]");
	return resultado;
}


process_t* buscar_proceso(int id_proceso){
    // esta funcion esta bien aca xd
    int key_search(process_t* un_proceso){
        return un_proceso->pid == id_proceso;
    }

    process_t* proceso_encontrado = list_find(PROCESS_TABLE, (void*)key_search);
    return proceso_encontrado;
}

