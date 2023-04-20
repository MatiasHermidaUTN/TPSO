#include "../include/planificacion_corto.h"


int contador_pid = 1;



t_pcb* crear_pcb(t_list* instrucciones, int socket_consola) {
	log_info(logger, "Antes del mutex en crear_pcb");
	t_pcb* pcb = malloc(sizeof(t_pcb));
	pthread_mutex_lock(&mutex_contador_pid);
	pcb->pid = contador_pid;
	contador_pid++;
	pthread_mutex_unlock(&mutex_contador_pid);

	pcb->instrucciones = instrucciones;
	pcb->pc =0;
	//pcb->registros_cpu = init_registros_cpu(); NO HACE FALTA INICIALIZARLOS, es memoria estatica
	pcb->tabla_segmentos = list_create();
	pcb->estimado_prox_rafaga = lectura_de_config.ESTIMACION_INICIAL;
	pcb->estimado_llegada_ready = 0;
	pcb->archivos_abiertos = list_create();

	pcb->socket_consola = socket_consola;

	return pcb;

}

void planificar_corto(void* args) {
	int socket_cpu = ((t_args_recibir_conexiones*)args)->socket_cliente;
	free(args);

	while(1) {
		sem_wait(&sem_ready); //entra solo si hay algun proceso en ready; es una espera no activa
		t_pcb* pcb = queue_pop_con_mutex(ready_queue, &mutex_ready_queue);

		log_info(logger, "Antes de enviar el pcb");
		enviar_pcb(pcb, socket_cpu); //Cómo sabe del socket_cpu?
		log_info(logger, "Despues de enviar el pcb");
		t_rta_cpu_al_kernel respuesta = esperar_cpu();

		switch(respuesta) {
			case YIELD: //vuelve a ready
				printf("Kernel recibio un YIELD por parte de la cpu");
				break;
			case IO: //bloquea
				printf("Kernel recibio un IO por parte de la cpu");
				break;
			case EXIT: //lo pone en exit
				printf("Kernel recibio un EXIT por parte de la cpu");
				break;
			default:
				log_error(logger,"Error en la comunicacion entre el Kernel y la CPU");
				exit(EXIT_FAILURE);
		}
	}
}

t_pcb *queue_pop_con_mutex(t_queue* queue, pthread_mutex_t* mutex) {
    pthread_mutex_lock(mutex);
    t_pcb* pcb = queue_pop(queue);
    pthread_mutex_unlock(mutex);
    return pcb;
}
