#include "../include/comunicaciones_memoria.h"

void manejar_conexion_kernel(int socket_kernel, t_log* logger){
	while(1){
		t_mensajes* mensaje = malloc(sizeof(t_mensajes));

		recibir_parametros(&mensaje);
		list_push_con_mutex(lista_fifo_msj, args, &mutex_cola_msj);
		sem_post(&sem_sincro_cant_msj);
	}
	return;
	//por ahi vendria bien agregar algun tipo de break para exitear el while
}

void manejar_conexion_cpu(int socket_cpu, t_log* logger){
	while(1){
		t_mensajes* mensaje = malloc(sizeof(t_mensajes));

		recibir_parametros(&mensaje);
		list_push_con_mutex(lista_fifo_msj, args, &mutex_cola_msj);
		sem_post(&sem_sincro_cant_msj);
	}
	return;
	//por ahi vendria bien agregar algun tipo de break para exitear el while
}

void manejar_conexion_fileSystem(int socket_fileSystem, t_log* logger){
	while(1){
		t_mensajes* mensaje = malloc(sizeof(t_mensajes));

		recibir_parametros(&mensaje);
		list_push_con_mutex(lista_fifo_msj, args, &mutex_cola_msj);
		sem_post(&sem_sincro_cant_msj);
	}
	return;
	//por ahi vendria bien agregar algun tipo de break para exitear el while
}

