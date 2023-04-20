#include "../include/planificacion_corto.h"

int contador_pid = 1;

t_pcb* crear_pcb(t_list* instrucciones,int socket_consola){
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

	pcb->socket_consola =socket_consola;

	return pcb;

}

void planificar_corto(){
	t_pcb* pcb;
	t_pcb* pcb_recibido;
	while(1){
		sem_wait(&sem_cant_ready); //entra solo si hay algun proceso en ready, es una espera no activa
		pcb = list_pop_con_mutex(ready_list, &mutex_ready_list);

		enviar_pcb(socket_cpu, pcb, PCB_A_EJECUTAR);
		print_l_instrucciones(pcb->instrucciones);
		liberar_pcb(pcb);


		t_msj_kernel_cpu rta = esperar_cpu();

		switch(rta){
			case YIELD_EJECUTADO: //vuelve a ready
				pcb_recibido = recibir_pcb(socket_cpu);
				list_push_con_mutex(ready_list, pcb_recibido, &mutex_ready_list);
				sem_post(&sem_cant_ready);
				break;
			case IO_EJECUTADO:
				break;
			case EXIT_EJECUTADO:
				pcb_recibido = recibir_pcb(socket_cpu);
				sem_post(&sem_multiprogramacion);
				enviar_fin_proceso(pcb_recibido->socket_consola, FINALIZACION_OK);
				//avisarle a memoria para liberar la estructura
				liberar_pcb(pcb_recibido);
				break;
			default:
				log_error(logger,"Error en la comunicacion entre el Kernel y la CPU");
				exit(EXIT_FAILURE);
		}
	}
}
