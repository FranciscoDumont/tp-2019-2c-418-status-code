#include "fuse_example.h"
#define PORT 8003



// Dentro de los argumentos que recibe nuestro programa obligatoriamente
// debe estar el path al directorio donde vamos a montar nuestro FS
int main(int argc, char *argv[]) {
	struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
	int socketfd;
	char* ip_server = "127.0.0.1";
	char mensaje[]= "estoy probando enviar mensajes";

	// Limpio la estructura que va a contener los parametros
	memset(&runtime_options, 0, sizeof(struct t_runtime_options));

	// Esta funcion de FUSE lee los parametros recibidos y los intepreta
	if (fuse_opt_parse(&args, &runtime_options, fuse_options, NULL) == -1){
		/** error parsing options */
		perror("Invalid arguments!");
		return EXIT_FAILURE;
	}

	// Si se paso el parametro --welcome-msg
	// el campo welcome_msg deberia tener el
	// valor pasado
	if( runtime_options.welcome_msg != NULL ){
		printf("%s\n", runtime_options.welcome_msg);
	}

	// Esta es la funcion principal de FUSE, es la que se encarga
	// de realizar el montaje, comuniscarse con el kernel, delegar todo
	// en varios threads

	if((socket_servidor = create_socket()) == -1) {
		printf("Error creating socket\n");
		return;
	}

	if(connect_socket(socket_servidor, ip_server, config->talk_port) == -1){
		printf("Error connecting to server\n");
		return;
	}

	t_paquete *package = crear_paquete(ABC);
	agregar_a_paquete(package, (void*) mensaje, strlen(mensaje) + 1);

	return fuse_main(args.argc, args.argv, &hello_oper, NULL);
}