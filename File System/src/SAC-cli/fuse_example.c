

#include "fuse_example.h"

bool server_socket_initialized = false; //Para que es?
int server_socket = 0;


//int sac_getattr(const char *ruta, struct stat *stbuf){
//    sac_cli_config* _config = read_config();
//
//    t_paquete *package = create_package(GETATTR);
//    add_to_package(package, (void*) ruta, strlen(ruta) + 1);
//
//    int resultado_envio =  send_package(package, server_socket);
//
//    int res = 0;
//
//    memset(stbuf, 0, sizeof(struct stat));
//
//    //Si path es igual a "/" nos estan pidiendo los atributos del punto de montaje
//
//    if (strcmp(ruta, "/") == 0) {
//        stbuf->st_mode = S_IFDIR | 0755;
//        stbuf->st_nlink = 2;
//    } else if (strcmp(ruta, DEFAULT_FILE_PATH) == 0) {
//        stbuf->st_mode = S_IFREG | 0444;
//        stbuf->st_nlink = 1;
//        stbuf->st_size = strlen(DEFAULT_FILE_CONTENT);
//    } else {
//        res = -ENOENT;
//    }
//
//
//    return res;
//}
//
//int sac_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
//    sac_cli_config* _config = read_config();
//
//    t_paquete *package = create_package(READDIR);
//    add_to_package(package, (void*) path, strlen(path) + 1);
//
//    int resultado_envio =  send_package(package, server_socket);
//
//
//    (void) offset;
//    (void) fi;
//
//    if (strcmp(path, "/") != 0)
//        return -ENOENT;
//
//    // "." y ".." son entradas validas, la primera es una referencia al directorio donde estamos parados
//    // y la segunda indica el directorio padre
//    filler(buf, ".", NULL, 0);
//    filler(buf, "..", NULL, 0);
//    filler(buf, DEFAULT_FILE_NAME, NULL, 0);
//
//    return 0;
//}
//
//
//int sac_open(const char *ruta, struct fuse_file_info *fi){
//    t_paquete *package = create_package(OPEN);
//    add_to_package(package, (void*) ruta, strlen(ruta) + 1);
//
//    int resultado_envio =  send_package(package, server_socket);
//
//    if (strcmp(ruta, DEFAULT_FILE_PATH) != 0)
//        return -ENOENT;
//
//    if ((fi->flags & 3) != O_RDONLY)
//        return -EACCES;
//
//    return 0;
//}
//
//int sac_mkdir(char* ruta){
//    t_paquete *package = create_package(MKDIR);
//    add_to_package(package, (void*) ruta, strlen(ruta) + 1);
//
//    int resultado_envio =  send_package(package, server_socket);
//
//    return resultado_envio == -1? -1 : resultado_envio;
//}
//
//int sac_mknod(char* ruta){
//    t_paquete *package = create_package(MKNOD);
//    add_to_package(package, (void*) ruta, strlen(ruta) + 1);
//
//    int resultado_envio =  send_package(package, server_socket);
//
//    return resultado_envio == -1? -1 : resultado_envio;
//}
//
//int sac_read(const char *ruta, char *buf, size_t size, off_t offset, struct fuse_file_info *fi){
//    t_paquete *package = create_package(READ);
//    add_to_package(package, (void*) ruta, strlen(ruta) + 1);
//
//    int resultado_envio =  send_package(package, server_socket);
//
//    size_t len;
//    (void) fi;
//    if (strcmp(ruta, DEFAULT_FILE_PATH) != 0)
//        return -ENOENT;
//
//    len = strlen(DEFAULT_FILE_CONTENT);
//    if (offset < len) {
//        if (offset + size > len)
//            size = len - offset;
//        memcpy(buf, DEFAULT_FILE_CONTENT + offset, size);
//    } else
//        size = 0;
//
//    return size;
//}
//
//int sac_write (const char * ruta, const char * algo, size_t size, off_t off_set, struct fuse_file_info * nosequees){
//    t_paquete *package = create_package(WRITE);
//    add_to_package(package, (void*) ruta, strlen(ruta) + 1);
//
//    int resultado_envio =  send_package(package, server_socket);
//
//    return resultado_envio == -1? -1 : resultado_envio;
//}

sac_cli_config* read_config(){
    sac_cli_config* config;
    t_log* logger;

    config = malloc(sizeof(sac_cli_config));

    logger = log_create("sac_cli.log", "sac-cli", 1, LOG_LEVEL_TRACE);


    t_config* config_file = config_create("sac_cli.config");
    config->talk_port = config_get_int_value(config_file, "LISTEN_PORT");
    config->ip = config_get_string_value(config_file, "IP");

    log_trace(logger,
              "Config file read: LISTEN_PORT: %d, IP: %s",
              config->talk_port,
              config->ip
    );

    return config;
}

void sac_init(){

    sac_cli_config* _config = read_config();

    if((server_socket = create_socket()) == -1) {
        printf("Error creating socket\n");
        return;
    }

    //conecto el socket
    if(-1 == connect_socket(server_socket, _config->ip, _config->talk_port)){
        printf("Error connect ::NOT FOUND %d \n", _config->talk_port);
    }else {
        printf("EL connect anda bien ::E \n");
    }

    server_socket_initialized = true;
}

// Dentro de los argumentos que recibe nuestro programa obligatoriamente
// debe estar el path al directorio donde vamos a montar nuestro FS
int main(int argc, char *argv[]) {

    sac_init();

    struct fuse_args args = FUSE_ARGS_INIT(argc, argv);

    // Limpio la estructura que va a contener los parametros
    memset(&runtime_options, 0, sizeof(struct t_runtime_options));

    // Esta funcion de FUSE lee los parametros recibidos y los intepreta
    if (fuse_opt_parse(&args, &runtime_options, fuse_options, NULL) == -1) {
        /** error parsing options */
        perror("Invalid arguments!");
        return EXIT_FAILURE;
    }

    // Si se paso el parametro --welcome-msg
    // el campo welcome_msg deberia tener el
    // valor pasado
    if (runtime_options.welcome_msg != NULL) {
        printf("%s\n", runtime_options.welcome_msg);
    }

    // Esta es la funcion principal de FUSE, es la que se encarga
    // de realizar el montaje, comuniscarse con el kernel, delegar todo
    // en varios threads

    return fuse_main(args.argc, args.argv, &hello_oper, NULL);
}


