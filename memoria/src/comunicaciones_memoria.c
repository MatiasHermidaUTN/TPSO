#include "../include/comunicaciones_memoria.h"

int recibir_conexiones() {
	int* socket_cliente = malloc(sizeof(int));
	*socket_cliente = esperar_cliente(socket_memoria);

	if(*socket_cliente != -1){
		pthread_t hilo;

		pthread_create(&hilo, NULL, (void*)manejar_conexion, socket_cliente);
		pthread_detach(hilo);

		return 1;
	}
	return 0;
}

void manejar_conexion(int* socket_cliente) {

	t_handshake rta_handshake = recibir_handshake(*socket_cliente);

	switch(rta_handshake){
		case KERNEL:
			log_warning(my_logger, "Kernel se conecto a Memoria");
			enviar_handshake(*socket_cliente, OK_HANDSHAKE);
			socket_kernel = *socket_cliente;
			free(socket_cliente);
			manejar_conexion_kernel();
			break;

		case CPU:
			log_warning(my_logger, "CPU se conecto a Memoria");
			enviar_handshake(*socket_cliente, OK_HANDSHAKE);
			socket_cpu = *socket_cliente;
			free(socket_cliente);
			manejar_conexion_cpu();
			break;

		case FILESYSTEM:
			log_warning(my_logger, "File System se conecto a Memoria");
			enviar_handshake(*socket_cliente, OK_HANDSHAKE);
			socket_fileSystem = *socket_cliente;
			free(socket_cliente);
			manejar_conexion_fileSystem();
			break;
			
		default:
			log_error(my_logger, "Error en el handshake");
			enviar_handshake(*socket_cliente, ERROR_HANDSHAKE);
			free(socket_cliente);
	}
}

void manejar_conexion_kernel(){
	while(1){
		t_mensajes* mensaje = malloc(sizeof(t_mensajes));

		mensaje->cod_op = recibir_msj(socket_kernel);
		mensaje->origen_mensaje = KERNEL;
		if(mensaje->cod_op != COMPACTAR){
			mensaje->parametros = recibir_parametros_de_mensaje(socket_kernel);
		}
		else{
			mensaje->parametros = string_array_new();
		}

		list_push_con_mutex(lista_fifo_msj, mensaje, &mutex_cola_msj);
		sem_post(&sem_cant_msj);
	}
	return;
	//TODO: por ahi vendria bien agregar algun tipo de break para exitear el while
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
	//TODO: por ahi vendria bien agregar algun tipo de break para exitear el while
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
	//TODO: por ahi vendria bien agregar algun tipo de break para exitear el while
}

