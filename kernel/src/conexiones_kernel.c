#include "../include/conexiones_kernel.h"

int recibir_conexiones(int socket_kernel) {
	int socket_consola = esperar_cliente(socket_kernel);
	if(socket_consola != -1) { //TODO: checkear -1 es error
		pthread_t hilo;
		t_args_recibir_conexiones* args = malloc(sizeof(t_args_recibir_conexiones)); //tiene que ser puntero?
		args->socket_cliente = socket_consola;

		pthread_create(&hilo, NULL, (void*)manejar_conexion, (void*)args);
		pthread_detach(hilo);

		return 1;
	}
	return 0;
}

void manejar_conexion(void* args) {
	int socket_consola = ((t_args_recibir_conexiones*)args)->socket_cliente;
	free(args);

	log_info(logger, "se conecto una consola");

	t_msj_kernel_consola cod_op = recibir_operacion(socket_consola);
	switch(cod_op) {
		case LIST_INSTRUCCIONES:
			log_info(logger, "Consola mando instrucciones");
			t_list* instrucciones = recibir_instrucciones(socket_consola);
			t_pcb* pcb = crear_pcb(instrucciones, socket_consola);

			log_info(logger,"Se crea el proceso %d en NEW \n",pcb->pid);
			queue_push_con_mutex(new_queue, pcb, &mutex_new_queue);

			sem_post(&sem_cant_new);

			break;
		default:
			log_error(logger, "Error en el envio de instrucciones");
	}
}

int contador_pid = 1;

t_pcb* crear_pcb(t_list* instrucciones,int socket_consola) {
	t_pcb* pcb = malloc(sizeof(t_pcb));
	pthread_mutex_lock(&mutex_contador_pid);
	pcb->pid = contador_pid;
	contador_pid++;
	pthread_mutex_unlock(&mutex_contador_pid);

	pcb->instrucciones = instrucciones;
	pcb->pc =0;
	//pcb->registros_cpu = init_registros_cpu(); NO HACE FALTA INICIALIZARLOS, es memoria estatica
	pcb->tabla_segmentos = list_create();
	pcb->estimado_prox_rafaga = lectura_de_config.ESTIMACION_INICIAL;
	pcb->tiempo_llegada_ready = 0;
	pcb->archivos_abiertos = list_create();

	pcb->socket_consola = socket_consola;

	pcb->tiempo_real_ejecucion = 0; // No hace falta igual, porque no se usa hasta que sea modificado por la CPU
	pcb->tiempo_inicial_ejecucion = 0;

	return pcb;

}

void init_conexiones(t_kernel_config lectura_de_config, t_log* logger, int* socket_memoria, int* socket_cpu, int* socket_fileSystem) {
	*socket_memoria = crear_conexion(lectura_de_config.IP_MEMORIA, lectura_de_config.PUERTO_MEMORIA);
	if(*socket_memoria == -1) {
		log_error(logger, "El kernel no pudo conectarse a la memoria");
		exit(EXIT_FAILURE);
	}
	enviar_handshake(*socket_memoria, KERNEL);
	t_handshake rta = recibir_handshake(*socket_memoria);
	if(rta == ERROR_HANDSHAKE){
		log_error(logger,"El kernel no se pudo conectar a la memoria");
		exit(EXIT_FAILURE);
	}

	//Para estos no hace falta handshake porque solo reciben al Kernel
	*socket_cpu = crear_conexion(lectura_de_config.IP_CPU, lectura_de_config.PUERTO_CPU);
	if(*socket_cpu == -1) {
		log_error(logger, "El kernel no pudo conectarse a el CPU");
		exit(EXIT_FAILURE);
	}

	*socket_fileSystem = crear_conexion(lectura_de_config.IP_FILESYSTEM, lectura_de_config.PUERTO_FILESYSTEM);
	if(*socket_fileSystem == -1) {
		log_error(logger, "El kernel no pudo conectarse a el FileSystem");
		exit(EXIT_FAILURE);
	}
}
