#include "muse.h"


int main() {
    uint32_t tam = 2;
    //uint32_t *dir = muse_alloc(tam);
    printf("Estoy imprimiendo el valor de tam: %d\n", tam);
    //printf("Estoy imprimiendo el valor de dir: %d\n", dir);
    //muse_free(&dir);
    read_memory_config();

    return 0;
}


void read_memory_config(){
    config_file = config_create("config");

    config.listen_port = config_get_int_value(config_file, "LISTEN_PORT");
    config.memory_size = config_get_int_value(config_file, "MEMORY_SIZE");
    config.page_size = config_get_int_value(config_file, "PAGE_SIZE");
    config.swap_size = config_get_int_value(config_file, "SWAP_SIZE");

    printf("LISTEN_PORT: %d\nMEMORY_SIZE: %d\nPAGE_SIZE: %d\nSWAP_SIZE: %d",\
        config.listen_port, \
        config.memory_size, \
        config.page_size, \
        config.swap_size);
}
