#include "../include/escuchador_de_memoria.h"

void log_acceso_memoria() {
	while(1) {
		t_msj_memoria instruccion_realizada = recibir_msj(socket_memoria);
		char** parametros_memoria = recibir_parametros_de_mensaje(socket_memoria);

		t_args_log_acceso_memoria* args = queue_pop_con_mutex(queue_solicitudes_acceso_memoria, mutex_queue_solicitudes_acceso_memoria);
		//No hace falta buscar por pid, porque total el que va a llegar (terminar de leer/escribir) siempre va a ser el primero (porque se ejecutan secuencialmente)

		char* accion;
		if(instruccion_realizada == LEIDO_OK) { //Lo hago con if y no con switch directamente para que no me tire un warning bíblico
			accion = strdup("LEER");
		}
		if(instruccion_realizada == ESCRITO_OK) {
			accion = strdup("ESCRIBIR");
		}

		char* valor = parametros_memoria[2];

		log_info(logger, "PID: %d - Acción: %s - Segmento: %d - Dirección Física: %d - Valor: %s", args->pid, accion, args->numero_segmento, args->direccion_fisica, valor); //log obligatorio

		free(accion);
		free(args);
	}
}
