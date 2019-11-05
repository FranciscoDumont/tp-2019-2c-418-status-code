#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <altaLibreria/connections.h>
#include <altaLibreria/structures.h>

#include <libmuse.h>


int main() {
    t_log* logger = log_create("tests.log", "TESTS", 1, LOG_LEVEL_TRACE);

    log_info(logger, "holaa");

    int respuesta = muse_init(114, "localhost", 5003);
    log_info(logger, "El resultado de muse_init es: %d\n\n", respuesta);

    uint32_t respuesta_alloc = muse_alloc(88);
    log_info(logger, "El resultado de muse_alloc es: %d\n\n", respuesta_alloc);

    log_destroy(logger);
    return 0;
}
