#include "fuse_example.h"

bool server_socket_initialized = false;
int server_socket = 0;

// Dentro de los argumentos que recibe nuestro programa obligatoriamente
// debe estar el path al directorio donde vamos a montar nuestro FS
//int main(int argc, char *argv[]) {

//	 struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
//
//	// Limpio la estructura que va a contener los parametros
//	memset(&runtime_options, 0, sizeof(struct t_runtime_options));
//
//	// Esta funcion de FUSE lee los parametros recibidos y los intepreta
//	if (fuse_opt_parse(&args, &runtime_options, fuse_options, NULL) == -1) {
//		/** error parsing options */
//		perror("Invalid arguments!");
//		return EXIT_FAILURE;
//	}
//
//	// Si se paso el parametro --welcome-msg
//	// el campo welcome_msg deberia tener el
//	// valor pasado
//	if (runtime_options.welcome_msg != NULL) {
//		printf("%s\n", runtime_options.welcome_msg);
//	}
//
//	// Esta es la funcion principal de FUSE, es la que se encarga
//	// de realizar el montaje, comuniscarse con el kernel, delegar todo
//	// en varios threads
//
//	//return fuse_main(args.argc, args.argv, &hello_oper, NULL);


int main(){

	sac_init();

	//Recibo datos del servidor
	//Primero recibo el encabezado y el tamanio ya que son datos de tamaño fijo
	MessageHeader* buffer_header = malloc(sizeof(MessageHeader));
	if(-1 == receive_header(server_socket, buffer_header)){
		printf("Error al recibir header ::NOT FOUND\n");
	}else {
		printf("Header recibido:\n type: %d\n size : %d ::E\n", buffer_header->type,buffer_header->data_size);
	}

	t_list *cosas = receive_package(server_socket, buffer_header);

	switch (buffer_header->type){
		case ABC:
		{
			;
			char *mensaje = (char*)list_get(cosas, 0);
			printf("Mensaje recibido:%s\n", mensaje);
			break;
		}
		default:
		{
			printf("Operacion desconocida. No quieras meter la pata");
			break;
		}
	}

	//Libero el socket
	close_socket(server_socket);

	//free(buffer_data);
	//free(mensaje);

	return 0;
}

sac_cli_config* read_config(){
	sac_cli_config* config;
	t_log* logger;

	config = malloc(sizeof(sac_cli_config));

	logger = log_create("../sac_cli.log", "sac-cli", 1, LOG_LEVEL_TRACE);


	t_config* config_file = config_create("../sac_cli.config");
	config->talk_port = config_get_int_value(config_file, "LISTEN_PORT");
	config->ip = config_get_string_value(config_file, "IP");

	log_trace(logger,
			  "Config file read: LISTEN_PORT: %d, IP: %s",
			  config->talk_port,
			  config->ip
	);
	config_destroy(config_file);

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