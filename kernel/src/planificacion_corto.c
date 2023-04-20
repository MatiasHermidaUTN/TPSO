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
	while(1){
		sem_wait(&sem_cant_ready); //entra solo si hay algun proceso en ready, es una espera no activa
		t_pcb* pcb = list_pop_con_mutex(ready_list, &mutex_ready_list);
		enviar_pcb(pcb);

		print_l_instrucciones(pcb->instrucciones);


		t_msj_kernel_cpu rta = esperar_cpu();

		switch(rta){
			case YIELD_EJECUTADO: //vuelve a ready
				break;
			case IO_EJECUTADO: //bloquea
				break;
			case EXIT_EJECUTADO: //lo pone en exit
				sem_post(&sem_multiprogramacion);
				break;
			default:
				log_error(logger,"Error en la comunicacion entre el Kernel y la CPU");
				exit(EXIT_FAILURE);
		}
	}
}
