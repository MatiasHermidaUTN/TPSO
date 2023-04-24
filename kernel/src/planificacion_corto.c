#include "../include/planificacion_corto.h"

void planificar_corto(){
	t_pcb* pcb;
	t_pcb* pcb_recibido;
	while(1){
		sem_wait(&sem_cant_ready); //entra solo si hay algun proceso en ready, es una espera no activa
		pcb = obtener_proximo_a_ejecutar();

		enviar_pcb(socket_cpu, pcb, PCB_A_EJECUTAR);
		print_l_instrucciones(pcb->instrucciones);
		liberar_pcb(pcb);


		t_msj_kernel_cpu rta = esperar_cpu();

		switch(rta){
			case YIELD_EJECUTADO: //vuelve a ready
				pcb_recibido = recibir_pcb(socket_cpu);
				pcb_recibido->tiempo_llegada_ready = time(NULL);
				list_push_con_mutex(ready_list, pcb_recibido, &mutex_ready_list);
				sem_post(&sem_cant_ready);
				break;
			case IO_EJECUTADO:
				char* parametro = recibir_parametro();
				int tiempo = atoi(parametro);
				free(parametro);
				pcb_recibido = recibir_pcb(socket_cpu);

				t_args_io* args = malloc(sizeof(t_args_io));
			    args->tiempo = tiempo;
				args->pcb = pcb_recibido;

				pthread_t hilo_io;
				pthread_create(&hilo_io, NULL, (void*)manejar_io, (void*)args);
				pthread_detach(hilo_io);
				break;

			case EXIT_EJECUTADO:
				pcb_recibido = recibir_pcb(socket_cpu);
				log_info(logger,"AX: %s",pcb_recibido->registros_cpu.AX);
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

void calcular_prox_rafaga(t_pcb* pcb){
	int alpha = lectura_de_config.HRRN_ALFA;
	pcb->estimado_prox_rafaga = alpha * pcb->tiempo_real_ejecucion + (1-alpha) * pcb->estimado_prox_rafaga; //pcb->estimado_prox_rafaga aca es como el estimado anterior
}

int calcular_R(t_pcb* pcb){
	return (calcular_tiempo_en_ready(pcb->tiempo_llegada_ready) + pcb->estimado_prox_rafaga) / pcb->estimado_prox_rafaga;
}

int calcular_tiempo_en_ready(int segundos){
	return time(NULL) - segundos;
}

t_pcb* obtener_proximo_a_ejecutar(){
	if(!strcmp(lectura_de_config.ALGORITMO_PLANIFICACION,"FIFO")){
		return list_pop_con_mutex(ready_list, &mutex_ready_list);
	}
	else if (!strcmp(lectura_de_config.ALGORITMO_PLANIFICACION,"HRRN")){
		pthread_mutex_lock(&mutex_ready_list);
		list_iterate(ready_list,(void*)calcular_prox_rafaga); //map con efecto
		t_pcb* pcb = list_get_maximum(ready_list, (void*)calcular_R);
		list_remove_element(ready_list,pcb); //esta funcion esta en github pero aca no la reconoce.
		pthread_mutex_unlock(&mutex_ready_list);
		return pcb;
	}
	else log_error(logger,"Error en la lectura del algoritmo de planificacion");
	exit(2);
}

int list_remove_element(t_list *self, void *element) {//la copiamos de las commons porque por algun motivo aca no la reconoce.
	int _is_the_element(void *data) {
		return element == data;
	}
	return list_remove_by_condition(self, (void*)_is_the_element) != NULL;
}

void manejar_io(t_args_io* args_io){
	sleep(args_io->tiempo);
	list_push_con_mutex(ready_list,args_io->pcb , &mutex_ready_list);
	sem_post(&sem_cant_ready);
	free(args_io);
}
