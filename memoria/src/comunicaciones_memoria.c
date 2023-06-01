#include "../include/comunicaciones_memoria.h"

void manejar_conexion_kernel(){
	while(1){
		t_mensajes* mensaje = malloc(sizeof(t_mensajes));

		mensaje->cod_op = recibir_msj(socket_kernel);
		mensaje->origen_mensaje = KERNEL;
		mensaje->parametros = recibir_parametros_de_mensaje(socket_kernel);
		list_push_con_mutex(lista_fifo_msj, mensaje, &mutex_cola_msj);
		sem_post(&sem_cant_msj);
	}
	return;
	//por ahi vendria bien agregar algun tipo de break para exitear el while
}

void manejar_conexion_cpu(){
	while(1){
		t_mensajes* mensaje = malloc(sizeof(t_mensajes));

		mensaje->cod_op = recibir_msj(socket_cpu);
		mensaje->origen_mensaje = CPU;
		mensaje->parametros = recibir_parametros_de_mensaje(socket_cpu);
		list_push_con_mutex(lista_fifo_msj, mensaje, &mutex_cola_msj);
		sem_post(&sem_cant_msj);
	}
	return;
	//por ahi vendria bien agregar algun tipo de break para exitear el while
}

void manejar_conexion_fileSystem(){
	while(1){
		t_mensajes* mensaje = malloc(sizeof(t_mensajes));

		mensaje->cod_op = recibir_msj(socket_fileSystem);
		mensaje->origen_mensaje = FILESYSTEM;
		mensaje->parametros = recibir_parametros_de_mensaje(socket_fileSystem);
		list_push_con_mutex(lista_fifo_msj, mensaje, &mutex_cola_msj);
		sem_post(&sem_cant_msj);
	}
	return;
	//por ahi vendria bien agregar algun tipo de break para exitear el while
}

int recibir_conexiones() {
	int socket_cliente = esperar_cliente(socket_memoria);
	if(socket_cliente != -1){ 
		pthread_t hilo;

		pthread_create(&hilo, NULL, (void*)manejar_conexion, &socket_cliente);
		pthread_detach(hilo);

		return 1;
	}
	return 0;
}

void manejar_conexion(int* socket_cliente) {

	t_handshake rta_handshake = recibir_handshake(*socket_cliente);
	switch(rta_handshake){
		case KERNEL:
			log_info(logger_no_obligatorio, "El Kernel se conecto a memoria");
			enviar_handshake(*socket_cliente, OK_HANDSHAKE);
			socket_kernel = *socket_cliente;
			manejar_conexion_kernel();
			break;

		case CPU:
			log_info(logger_no_obligatorio, "La CPU se conecto a memoria");
			enviar_handshake(*socket_cliente, OK_HANDSHAKE);
			socket_cpu = *socket_cliente;
			manejar_conexion_cpu();
			break;

		case FILESYSTEM:
			log_info(logger_no_obligatorio, "El Filesystem se conecto a memoria");
			enviar_handshake(*socket_cliente, OK_HANDSHAKE);
			socket_fileSystem = *socket_cliente;
			manejar_conexion_fileSystem();
			break;
			
		default:
			log_error(logger_no_obligatorio, "Error en el handshake");
			enviar_handshake(*socket_cliente, ERROR_HANDSHAKE);
	}

}

