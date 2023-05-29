#include "../include/planificacion_largo.h"

void planificar_largo() {
	while(1) {
		sem_wait(&sem_cant_new); //espera a que haya alguien en New
		sem_wait(&sem_multiprogramacion);

		t_pcb* pcb = queue_pop_con_mutex(new_queue, &mutex_new_queue);
		ready_list_push(pcb);

		//TODO: avisarle a memoria para que inicialice sus estructuras
	}
}
