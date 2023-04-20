#include "../include/conexiones_kernel.h"


int recibir_conexiones(int socket_kernel) {
	log_info(logger, "En recibir_conexiones del Kernel");
	int socket_consola = esperar_cliente(socket_kernel);
	if(socket_consola != -1) { //TODO: checkear -1 es error
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

void manejar_conexion(void* args) {
	int socket_consola = ((t_args_recibir_conexiones*)args)->socket_cliente;
	t_log* logger = ((t_args_recibir_conexiones*)args)->logger;
	free(args);

	log_info(logger, "se conecto una consola");

	op_code cod_op = recibir_operacion(socket_consola);
	switch(cod_op) {
		case LIST_INSTRUCCIONES:
			log_info(logger, "Consola mandando instrucciones");

			t_list* instrucciones = recibir_instrucciones(socket_consola);
			print_l_instrucciones(instrucciones);

			t_pcb* pcb = crear_pcb(instrucciones, socket_consola);

			log_info(logger, "Se crea el proceso %d en NEW\n", pcb->pid); //log obligatorio

			queue_push(ready_queue, pcb);
			sem_post(&sem_ready);

			//enviar_fin_proceso(socket_consola);
			break;
		default:
			log_error(logger, "Error en el handshake");
	}
}

void init_conexiones(t_kernel_config lectura_de_config, t_log* logger, int* socket_memoria, int* socket_cpu, int* socket_fileSystem){
	*socket_memoria = crear_conexion(lectura_de_config.IP_MEMORIA, lectura_de_config.PUERTO_MEMORIA);
	if(*socket_memoria == -1){
		log_error(logger, "El kernel no pudo conectarse a la memoria");
		exit(EXIT_FAILURE);
	}

	enviar_handshake(*socket_memoria, KERNEL);
	t_handshake respuesta = recibir_handshake(*socket_memoria);
	if(respuesta == ERROR){
		log_error(logger,"El kernel no se pudo conectar a la memoria");
		exit(EXIT_FAILURE);
	}

	//Para estos no hace falta handshake porque solo reciben al Kernel
	*socket_cpu = crear_conexion(lectura_de_config.IP_CPU, lectura_de_config.PUERTO_CPU);
	if(*socket_cpu == -1) {
		log_error(logger, "El kernel no pudo conectarse al CPU");
		exit(EXIT_FAILURE);
	}

	*socket_fileSystem = crear_conexion(lectura_de_config.IP_FILESYSTEM, lectura_de_config.PUERTO_FILESYSTEM);
	if(*socket_fileSystem == -1) {
		log_error(logger, "El kernel no pudo conectarse al FileSystem");
		exit(EXIT_FAILURE);
	}
}

void destruir_parametro(char* parametro){
	free(parametro);
}

void destruir_instruccion(t_instruccion* instruccion){
	free(instruccion->nombre);
	list_destroy_and_destroy_elements(instruccion->parametros, (void*)destruir_parametro);
	free(instruccion);
}
