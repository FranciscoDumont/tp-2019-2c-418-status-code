#include "muse.h"


int main() {
    logger = log_create("muse.log", "MUSE", 1, LOG_LEVEL_TRACE);
    read_memory_config();

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
