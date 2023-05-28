#include "../include/comunicaciones_memoria.h"

void manejar_conexion_kernel(int socket_kernel, t_log* logger){
	while(1){
		t_mensajes* mensaje = malloc(sizeof(t_mensajes));

		mensaje->cod_op = recibir_msj(socket_kernel);
		mensaje->origen_mensaje = KERNEL;
		//mensaje->parametros = recibir_parametros_de_mensaje(socket_kernel);
		list_push_con_mutex(lista_fifo_msj, mensaje, &mutex_cola_msj);
		sem_post(&sem_cant_msj);
	}
	return;
	//por ahi vendria bien agregar algun tipo de break para exitear el while
}

void manejar_conexion_cpu(int socket_cpu, t_log* logger){
	while(1){
		t_mensajes* mensaje = malloc(sizeof(t_mensajes));

		mensaje->cod_op = recibir_msj(socket_cpu);
		mensaje->origen_mensaje = CPU;
		//mensaje->parametros = recibir_parametros_de_mensaje(socket_cpu);
		list_push_con_mutex(lista_fifo_msj, mensaje, &mutex_cola_msj);
		sem_post(&sem_cant_msj);
	}
	return;
	//por ahi vendria bien agregar algun tipo de break para exitear el while
}

void manejar_conexion_fileSystem(int socket_fileSystem, t_log* logger){
	while(1){
		t_mensajes* mensaje = malloc(sizeof(t_mensajes));

		mensaje->cod_op = recibir_msj(socket_fileSystem);
		mensaje->origen_mensaje = FILESYSTEM;
		//mensaje->parametros = recibir_parametros_de_mensaje(socket_fileSystem);
		list_push_con_mutex(lista_fifo_msj, mensaje, &mutex_cola_msj);
		sem_post(&sem_cant_msj);
	}
	return;
	//por ahi vendria bien agregar algun tipo de break para exitear el while
}

