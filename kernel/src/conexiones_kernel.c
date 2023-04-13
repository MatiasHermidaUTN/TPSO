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
}


