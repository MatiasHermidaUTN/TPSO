#include "../include/conexiones_kernel.h"

int recibir_conexiones(int socket_kernel, t_log* logger){
	int socket_consola = esperar_cliente(socket_kernel);
	if(socket_consola != -1){ //TODO: checkear -1 es error
		pthread_t hilo;
		t_args_recibir_conexiones* args = malloc(sizeof(t_args_recibir_conexiones)); //tiene que ser puntero?
		args->socket_cliente = socket_consola;
		args->logger = logger;

		pthread_create(&hilo, NULL, (void*)manejar_conexion, (void*)args);
		pthread_detach(hilo);

		return 1;
	}
	return 0;
}

void manejar_conexion(void* args){
	int socket_consola = ((t_args_recibir_conexiones*)args)->socket_cliente;
	t_log* logger = ((t_args_recibir_conexiones*)args)->logger;
	free(args);

	log_info(logger, "se conecto una consola");


	//TODO: Implementar
	//recibir msj y deserializarlo -> ver comunicacion.c de consola
}

void init_conexiones(t_kernel_config lectura_de_config, t_log* logger, int* socket_memoria, int* socket_cpu, int* socket_fileSystem){
	*socket_memoria = crear_conexion(lectura_de_config.IP_MEMORIA, lectura_de_config.PUERTO_MEMORIA);
	if(*socket_memoria == -1){
		log_error(logger, "El kernel no pudo conectarse a la memoria");
		exit(EXIT_FAILURE);
	}
	enviar_handshake(*socket_memoria, KERNEL);

	//Para estos no hace falta handshake porque solo reciben al Kernel
	*socket_cpu = crear_conexion(lectura_de_config.IP_CPU, lectura_de_config.PUERTO_CPU);
	if(*socket_cpu == -1){
			log_error(logger, "El kernel no pudo conectarse a el CPU");
			exit(EXIT_FAILURE);
	}

	*socket_fileSystem = crear_conexion(lectura_de_config.IP_FILESYSTEM, lectura_de_config.PUERTO_FILESYSTEM);
	if(*socket_fileSystem == -1){
				log_error(logger, "El kernel no pudo conectarse a el FileSystem");
				exit(EXIT_FAILURE);
	}
}

