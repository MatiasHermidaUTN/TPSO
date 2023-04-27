#include "../include/planificacion_corto.h"

void planificar_corto() {
	t_pcb* pcb;
	t_pcb* pcb_recibido;
	while(1) {
		sem_wait(&sem_cant_ready); //entra solo si hay algun proceso en Ready, es una espera no activa
		pcb = obtener_proximo_a_ejecutar();

		enviar_pcb(socket_cpu, pcb, PCB_A_EJECUTAR, NULL); //NULL porque no se le pasa ningun parametro extra
		print_l_instrucciones(pcb->instrucciones);
		liberar_pcb(pcb);

		t_msj_kernel_cpu respuesta = esperar_cpu();

		switch(respuesta) {
			case YIELD_EJECUTADO: //vuelve a ready
				pcb_recibido = recibir_pcb(socket_cpu);
				ready_list_push(pcb_recibido); //Aca calcula el S (proxima rafaga) y actualizo el tiempo_llegada_ready (solo necesito pasarle el pcb, porque ya se que es en Ready)

				sem_post(&sem_cant_ready); //Dice que hay un nuevo proceso en Ready
				break;

			case IO_EJECUTADO:
				char* parametro = recibir_parametro_de_instruccion();
				int tiempo_a_bloquear = atoi(parametro);
				free(parametro);

				pcb_recibido = recibir_pcb(socket_cpu);
				//printf("El tiempo a bloquear de %d es: %d.\n", pcb_recibido->pid, tiempo_a_bloquear);

				t_args_io* args = malloc(sizeof(t_args_io));
			    args->tiempo = tiempo_a_bloquear;
				args->pcb = pcb_recibido;

				log_info(logger, "PID: %d - Estado Anterior: EXEC - Estado Actual: BLOCK", pcb_recibido->pid); //log obligatorio
				log_info(logger, "PID: %d - Bloqueado por: IO", pcb_recibido->pid); //log obligatorio
				log_info(logger, "PID: %d - Ejecuta IO: %d", pcb_recibido->pid, args->tiempo); //log obligatorio

				pthread_t hilo_io;
				pthread_create(&hilo_io, NULL, (void*)manejar_io, (void*)args);
				pthread_detach(hilo_io);
				break;

			case EXIT_EJECUTADO:
				pcb_recibido = recibir_pcb(socket_cpu);
				log_info(logger, "AX: %s",pcb_recibido->registros_cpu.AX);
				sem_post(&sem_multiprogramacion);
				enviar_fin_proceso(pcb_recibido->socket_consola, FINALIZACION_OK);

				log_info(logger, "PID: %d - Estado Anterior: EXEC - Estado Actual: EXIT", pcb_recibido->pid); //log obligatorio
				log_info(logger, "Finaliza el proceso %d - Motivo: SUCCESS", pcb_recibido->pid); //log obligatorio

				//avisarle a memoria para liberar la estructura
				liberar_pcb(pcb_recibido);
				break;

			default:
				log_error(logger,"Error en la comunicacion entre el Kernel y la CPU");
				exit(EXIT_FAILURE);
		}
	}
}

void ready_list_push(t_pcb* pcb_recibido) {
	pcb_recibido->tiempo_llegada_ready = time(NULL); //Actualizo el momento en que llega a Ready
	list_push_con_mutex(ready_list, pcb_recibido, &mutex_ready_list);

	list_iterate(ready_list, (void*)calcular_prox_rafaga); //map con efecto
}

void calcular_prox_rafaga(t_pcb* pcb) {
	int alpha = lectura_de_config.HRRN_ALFA;
	pcb->estimado_prox_rafaga = alpha * pcb->tiempo_real_ejecucion + (1-alpha) * pcb->estimado_prox_rafaga; //pcb->estimado_prox_rafaga aca es como el estimado anterior
	//printf("estimado_prox_rafaga de %d: %d.\n", pcb->pid, pcb->estimado_prox_rafaga);
}

int calcular_R(t_pcb* pcb) {
	return (calcular_tiempo_en_ready(pcb->tiempo_llegada_ready) + pcb->estimado_prox_rafaga) / pcb->estimado_prox_rafaga;
}

int calcular_tiempo_en_ready(int segundos){
	return time(NULL) - segundos;
}

t_pcb* obtener_proximo_a_ejecutar() {
	if(!strcmp(lectura_de_config.ALGORITMO_PLANIFICACION, "FIFO")) {
		return list_pop_con_mutex(ready_list, &mutex_ready_list);
	}
	else if (!strcmp(lectura_de_config.ALGORITMO_PLANIFICACION, "HRRN")) {
		pthread_mutex_lock(&mutex_ready_list);
		t_pcb* pcb = list_get_maximum(ready_list, (void*)calcular_R);
		list_remove_element(ready_list,pcb);
		pthread_mutex_unlock(&mutex_ready_list);
		return pcb;
	}
	else log_error(logger,"Error en la lectura del algoritmo de planificacion");
	exit(2);
}

void manejar_io(t_args_io* args_io) {
	sleep(args_io->tiempo);
	ready_list_push(args_io->pcb); //Aca calcula el S (proxima rafaga) y actualizo el tiempo_llegada_ready
	sem_post(&sem_cant_ready);
	free(args_io);
}
