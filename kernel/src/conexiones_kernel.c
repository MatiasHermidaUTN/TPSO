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

	log_warning(my_logger, "Se conecto una consola");

	t_msj_kernel_consola cod_op = recibir_msj(socket_consola);
	switch(cod_op) {
		case LIST_INSTRUCCIONES:
			t_list* instrucciones = recibir_instrucciones(socket_consola);
			t_pcb* pcb = crear_pcb(instrucciones, socket_consola);

			log_info(logger, "Se crea el proceso %d en NEW", pcb->pid); //log obligatorio
			queue_push_con_mutex(new_queue, pcb, &mutex_new_queue);

			sem_post(&sem_cant_new);

			break;
		default:
			log_error(logger, "Error en el envio de instrucciones");
	}
}

int contador_pid = 1;

t_pcb* crear_pcb(t_list* instrucciones, int socket_consola) {
	t_pcb* pcb = malloc(sizeof(t_pcb));

	pthread_mutex_lock(&mutex_contador_pid);
	pcb->pid = contador_pid;
	contador_pid++;
	pthread_mutex_unlock(&mutex_contador_pid);

	pcb->instrucciones = instrucciones;
	pcb->pc = 0;
	pcb->registros_cpu = init_registros_cpu();

	/*
	pthread_mutex_lock(&mutex_msj_memoria); TODO: DESCOMENTAR

	char** parametros_crear_pcb = string_array_new();
	string_array_push(&parametros_crear_pcb, string_itoa(pcb->pid));
	enviar_msj_con_parametros(socket_memoria, INICIALIZAR_PROCESO, parametros_crear_pcb);
	string_array_destroy(parametros_crear_pcb);

	if(recibir_msj(socket_memoria) == PROCESO_INICIALIZADO) { //No hace falta pero bueno, recibe un mensaje sí o sí
		pcb->tabla_segmentos = recibir_tabla_segmentos(socket_memoria);
	}
	else {
		log_error(logger, "Error en el uso de segmentos");
		exit(EXIT_FAILURE);
	}

	pthread_mutex_unlock(&mutex_msj_memoria);
	*/
	pcb->tabla_segmentos = list_create(); //TODO: SACAR!!

	pcb->estimado_prox_rafaga = lectura_de_config.ESTIMACION_INICIAL;
	pcb->tiempo_llegada_ready = 0;
	pcb->archivos_abiertos = list_create();

	pcb->socket_consola = socket_consola;

	pcb->tiempo_real_ejecucion = lectura_de_config.ESTIMACION_INICIAL; //Por si es la primera vez que entra en READY, para no afectar al calculo de la próxima ráfaga

	pcb->tiempo_inicial_ejecucion = 0;

	return pcb;
}

t_registros_cpu init_registros_cpu() {
	t_registros_cpu registros;

	/*
	for(int i = 0; i < 4; i++) {//TODO: usar memcpy
		registros.AX[i] = '0';
		registros.BX[i] = '0';
		registros.CX[i] = '0';
		registros.DX[i] = '0';
	}
	for(int i = 0; i < 8; i++) {
		registros.EAX[i] = '0';
		registros.EBX[i] = '0';
		registros.ECX[i] = '0';
		registros.EDX[i] = '0';
	}
	for(int i = 0; i < 16; i++) {
		registros.RAX[i] = '0';
		registros.RBX[i] = '0';
		registros.RCX[i] = '0';
		registros.RDX[i] = '0';
	}
	*/
	char* valor_inicial_4 = strdup("0000");
	memcpy(registros.AX, valor_inicial_4, 4*sizeof(char));
	memcpy(registros.BX, valor_inicial_4, 4*sizeof(char));
	memcpy(registros.CX, valor_inicial_4, 4*sizeof(char));
	memcpy(registros.DX, valor_inicial_4, 4*sizeof(char));
	free(valor_inicial_4);

	char* valor_inicial_8 = strdup("00000000");
	memcpy(registros.EAX, valor_inicial_8, 8*sizeof(char));
	memcpy(registros.EBX, valor_inicial_8, 8*sizeof(char));
	memcpy(registros.ECX, valor_inicial_8, 8*sizeof(char));
	memcpy(registros.EDX, valor_inicial_8, 8*sizeof(char));
	free(valor_inicial_8);

	char* valor_inicial_16 = strdup("0000000000000000");
	memcpy(registros.RAX, valor_inicial_16, 16*sizeof(char));
	memcpy(registros.RBX, valor_inicial_16, 16*sizeof(char));
	memcpy(registros.RCX, valor_inicial_16, 16*sizeof(char));
	memcpy(registros.RDX, valor_inicial_16, 16*sizeof(char));
	free(valor_inicial_16);

	return registros;
}

void init_conexiones() {
	//TODO: DESCOMENTAR
/*
	socket_memoria = crear_conexion(lectura_de_config.IP_MEMORIA, lectura_de_config.PUERTO_MEMORIA);
	if(socket_memoria == -1) {
		log_error(logger, "Kernel no pudo conectarse a Memoria");
		exit(EXIT_FAILURE);
	}

	enviar_handshake(socket_memoria, KERNEL);
	t_handshake respuesta = recibir_handshake(socket_memoria);
	if(respuesta == ERROR_HANDSHAKE){
		log_error(logger,"Kernel no pudo conectarse a Memoria");
		exit(EXIT_FAILURE);
	}
*/

	//Para estos no hace falta handshake porque solo reciben al Kernel
	socket_cpu = crear_conexion(lectura_de_config.IP_CPU, lectura_de_config.PUERTO_CPU);
	if(socket_cpu == -1) {
		log_error(logger, "Kernel no pudo conectarse a CPU");
		exit(EXIT_FAILURE);
	}

	//socket_fileSystem = crear_conexion(lectura_de_config.IP_FILESYSTEM, lectura_de_config.PUERTO_FILESYSTEM);
	//if(socket_fileSystem == -1) {
	//	log_error(logger, "Kernel no pudo conectarse a File System");
	//	exit(EXIT_FAILURE);
	//}
}
