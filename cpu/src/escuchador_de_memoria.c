#include "../include/escuchador_de_memoria.h"

void log_acceso_memoria() {
	/* TODO: DESCOMENTAR!!!! (cuando esté memoria lista)
	while(1) {
		t_msj_memoria instruccion_realizada = recibir_msj(socket_memoria); //Es bloqueante, por lo que me sirve como sistema productor-consumidor para la lista
		char** parametros_memoria = recibir_parametros_de_mensaje(socket_memoria); //pid + valor escrito/leido

		int pid = atoi(parametros_memoria[0]);

		pthread_mutex_lock(&mutex_list_solicitudes_acceso_memoria);
		t_args_log_acceso_memoria* args = list_remove_solicitud(pid);
		pthread_mutex_unlock(&mutex_list_solicitudes_acceso_memoria);

		//Hace falta buscar por pid, porque, aunque el que va a llegar (terminar de leer/escribir) siempre va a ser el primero (porque se ejecutan secuencialmente),
		//eso solo pasa con F_WRITE y F_READ, pero también uso MOV_IN y MOV_OUT para esto.


		char* accion;
		if(instruccion_realizada == LEIDO_OK) { //Lo hago con if y no con switch directamente para que no me tire un warning bíblico
			accion = strdup("LEER");
		}
		if(instruccion_realizada == ESCRITO_OK) {
			accion = strdup("ESCRIBIR");
		}
		else {
			log_error(logger, "Error en el uso de segmentos");
		}

		char* valor = parametros_memoria[1];
		log_info(logger, "PID: %d - Acción: %s - Segmento: %d - Dirección Física: %d - Valor: %s", pid, accion, args->numero_segmento, args->direccion_fisica, valor); //log obligatorio

		if(args->nombre_registro) { //Si se ejecutó MOV_IN
			set_registro(args->pcb, args->nombre_registro, valor);
		}

		string_array_destroy(parametros_memoria);
		free(accion);
		free(args);
	}
	*/
}

t_args_log_acceso_memoria* list_remove_solicitud(int pid) {
	t_args_log_acceso_memoria* args = NULL;

	for(int i = 0; i < list_size(list_solicitudes_acceso_memoria); i++) {
		args = list_get(list_solicitudes_acceso_memoria, i);
		if(args->pcb->pid == pid) {
			return list_remove(list_solicitudes_acceso_memoria, i);
		}
	}

	return args; //No debería llegar acá
}
