#include "../include/planificacion_largo.h"

void planificar_largo() {
	while(1) {
		sem_wait(&sem_cant_new); //espera a que haya alguien en New
		sem_wait(&sem_multiprogramacion);

		t_pcb* pcb = queue_pop_con_mutex(new_queue, &mutex_new_queue);

		pthread_mutex_lock(&mutex_msj_memoria);

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

		ready_list_push(pcb);
	}
}
