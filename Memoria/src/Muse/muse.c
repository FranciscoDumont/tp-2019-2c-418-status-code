#include "muse.h"


int main() {
    logger = log_create("muse.log", "MUSE", 1, LOG_LEVEL_TRACE);
    read_memory_config();

    process_table = list_create();
    segment_table = list_create();

    cantidad_paginas_actuales = 0;
    limite_paginas = config.memsize / obtener_tamanio_pagina();
    mapa_memoria_size = limite_paginas;
    mapa_memoria = calloc(limite_paginas,sizeof(int));
    memoria_principal = malloc(config.memsize);
    custom_print("Iniciando memoria de %d paginas\n", limite_paginas);
    log_info(logger, "Se pueden almacenar %d páginas", limite_paginas);

    return 0;
}


void read_memory_config(){
    config_file = config_create("config");

    config.listen_port = config_get_int_value(config_file, "LISTEN_PORT");
    config.memory_size = config_get_int_value(config_file, "MEMORY_SIZE");
    config.page_size = config_get_int_value(config_file, "PAGE_SIZE");
    config.swap_size = config_get_int_value(config_file, "SWAP_SIZE");

    log_info(logger, \
        "Configuración levantada\n\tLISTEN_PORT: %d\n\tMEMORY_SIZE: %d\n\tPAGE_SIZE: %d\n\tSWAP_SIZE: %d.",\
        config.listen_port, \
        config.memory_size, \
        config.page_size, \
        config.swap_size);
}
