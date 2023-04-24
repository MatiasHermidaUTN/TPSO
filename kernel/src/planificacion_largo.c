#include "../include/planificacion_largo.h"

void planificar_largo(){
	while(1){
		sem_wait(&sem_cant_new); //para que no haga nada si no hay nadie en new
		sem_wait(&sem_multiprogramacion);
		if(queue_size(new_queue) > 0){
			t_pcb* pcb = queue_pop_con_mutex(new_queue, &mutex_new_queue);
			pcb->tiempo_llegada_ready = time(NULL);
			list_push_con_mutex(ready_list,pcb, &mutex_ready_list);
			log_info(logger, "PID: %d - Estado Anterior: NEW - Estado Actual: READY", pcb->pid);
			//log_info(logger, "Cola Ready %s: [%s]", lectura_de_config.ALGORITMO_PLANIFICACION, obtener_pids(ready_list));
			sem_post(&sem_cant_ready);
		}
	}
}
