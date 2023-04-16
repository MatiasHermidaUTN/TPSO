#include "../include/conexiones_memoria.h"
#include "../include/comunicaciones_memoria.h"

int recibir_conexiones(int socket_memoria, t_log* logger) {
	int socket_cliente = esperar_cliente(socket_memoria);
	if(socket_cliente != -1) { //TODO: checkear -1 es error
		pthread_t hilo;
		t_args_recibir_conexiones* args = malloc(sizeof(t_args_recibir_conexiones)); //tiene que ser puntero?
		args->socket_cliente = socket_cliente;
		args->logger = logger;

		pthread_create(&hilo, NULL, (void*)manejar_conexion, (void*)args);
		pthread_detach(hilo);

		return 1;
	}
	return 0;
}

void manejar_conexion(void* args) {
	int socket_cliente = ((t_args_recibir_conexiones*)args)->socket_cliente;
	t_log* logger = ((t_args_recibir_conexiones*)args)->logger;
	free(args);

	t_handshake respuesta_handshake = recibir_handshake(socket_cliente);
	switch(respuesta_handshake) {
		case KERNEL:
			log_info(logger, "El Kernel se conecto a Memoria");
			manejar_conexion_kernel(socket_cliente, logger);
			break;
		case CPU:
			log_info(logger, "La CPU se conecto a Memoria");
			manejar_conexion_cpu(socket_cliente, logger);
			break;
		case FILESYSTEM:
			log_info(logger, "El File System se conecto a Memoria");
			manejar_conexion_fileSystem(socket_cliente, logger);
			break;
		default:
			log_error(logger, "Error en el handshake");
	}

}
