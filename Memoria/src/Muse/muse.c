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
//agrego la implementacion de memcpy porque no reconoce la original
void * memcpy(void* dst, const void* src, unsigned long tam)
{
    char *toBeCopied = (char *)dst;
    const char *original =( const char*)src;
    while(tam)
    {
        *(toBeCopied++)= *(original++);
        --tam;
    }
    return dst;
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
    log_info(logger, "El ip que recibo de libMuse es: %d", ip);
    log_info(logger, "El puerto que recibo de libMuse es: %d", puerto);

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
    int frame_libre = mp_buscar_frame_libre();
    page_t* nueva_pagina = crear_pagina(frame_libre, true, false, false); //todo ver si estan bien los bits esos
    segment_t* nuevo_segmento = crear_segmento(false);
    list_add(nuevo_segmento->pages, nueva_pagina);
    list_add(el_proceso->segments, nuevo_segmento);
    void* espacio_libre = MAIN_MEMORY + config.page_size * frame_libre;
    mp_escribir_metadata(espacio_libre, tam, false);
    mp_escribir_metadata(espacio_libre + tam, (uint32_t)paginas_necesarias-tam, true);
    MAPA_MEMORIA[frame_libre] = 1;

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


segment_t* crear_segmento(bool is_shared){
    segment_t* nuevo_segmento = malloc(sizeof(segment_t));
    nuevo_segmento->is_shared = is_shared;
    t_list* nueva_lista = list_create();
    nuevo_segmento->pages = nueva_lista;
    return nuevo_segmento;
}


page_t* crear_pagina(int frame, int presence_bit, int modified_bit, int use_bit){
    page_t* nueva_pagina = malloc(sizeof(page_t));
    nueva_pagina->frame = frame;
    nueva_pagina->presence_bit = presence_bit;
    nueva_pagina->modified_bit = modified_bit;
    nueva_pagina->use_bit = use_bit;
    return nueva_pagina;
}

void* mp_escribir_metadata(void* espacio_libre, uint32_t tam, int esta_libre){
    heap_metadata* nueva_metadata = malloc(sizeof(heap_metadata));
    nueva_metadata->size = tam;
    nueva_metadata->isFree = esta_libre;
    const void * metadata = nueva_metadata;
    memcpy(espacio_libre, metadata, sizeof(heap_metadata));
    return espacio_libre + sizeof(heap_metadata);
}


int mp_buscar_frame_libre(){
	int i;
	int nro_frame = -1; // Si no encuentra un frame libre va a devolver -1
	bool esta_todo_ok = false;
	for (i = 0; i<MAPA_MEMORIA_SIZE; ++i){
	    if (MAPA_MEMORIA[i] == 0){
	        nro_frame = i;
            break; //si el lugar está libre salgo del for
	    }
	}
    return nro_frame;
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
    bool key_search(void * un_proceso){
        process_t * process = (process_t *) un_proceso;
        return process->pid == id_proceso;
    }

    process_t* proceso_encontrado = list_find(PROCESS_TABLE, key_search);
    return proceso_encontrado;
    // esta funcion esta bien aca xd
}

