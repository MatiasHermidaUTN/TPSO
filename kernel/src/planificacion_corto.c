#include "../include/planificacion_corto.h"

void planificar_corto() {
	t_pcb* pcb;
	t_pcb* pcb_recibido;
	while(1) {
		sem_wait(&sem_cant_ready); //entra solo si hay algun proceso en Ready, es una espera no activa
		pcb = obtener_proximo_a_ejecutar();
		log_warning(logger, "Proximo a ejecutar: %d", pcb->pid);

		enviar_pcb(socket_cpu, pcb, PCB_A_EJECUTAR, NULL); //NULL porque no se le pasa ningun parametro extra
		liberar_pcb(pcb);

		t_msj_kernel_cpu respuesta = esperar_cpu();

		switch(respuesta) {
			case IO_EJECUTADO:
				char* parametro = recibir_parametro_de_instruccion();
				int tiempo_a_bloquear = atoi(parametro);
				free(parametro);

				pcb_recibido = recibir_pcb(socket_cpu);

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

			case WAIT_EJECUTADO:
				char* nombre_recurso_wait = recibir_parametro_de_instruccion();
				pcb_recibido = recibir_pcb(socket_cpu);
				wait_recurso(pcb_recibido, nombre_recurso_wait);
				break;

			case SIGNAL_EJECUTADO:
				char* nombre_recurso_signal = recibir_parametro_de_instruccion();
				pcb_recibido = recibir_pcb(socket_cpu);
				signal_recurso(pcb_recibido, nombre_recurso_signal);
				break;

			case YIELD_EJECUTADO: //vuelve a ready
				pcb_recibido = recibir_pcb(socket_cpu);
				ready_list_push(pcb_recibido); //Aca calcula el S (proxima rafaga), actualizo el tiempo_llegada_ready y hago sem_post(&sem_cant_ready) (solo necesito pasarle el pcb, porque ya se que es en Ready)
				log_info(logger, "PID: %d - Estado Anterior: EXEC - Estado Actual: READY", pcb_recibido->pid); //log obligatorio
				break;

			case EXIT_EJECUTADO:
				pcb_recibido = recibir_pcb(socket_cpu);
				exit_proceso(pcb_recibido); //Aca hace el sem_post(&sem_multiprogramacion)
				break;

			default:
				log_error(logger,"Error en la comunicacion entre el Kernel y la CPU");
				exit(1);
		}
	}
}

void ready_list_push(t_pcb* pcb_recibido) {
	pcb_recibido->tiempo_llegada_ready = time(NULL); //Actualizo el momento en que llega a Ready
	calcular_prox_rafaga(pcb_recibido); //S se calcula solo al que entra a Ready
	list_push_con_mutex(ready_list, pcb_recibido, &mutex_ready_list);
	sem_post(&sem_cant_ready); //Dice que hay un nuevo proceso en Ready
}

void calcular_prox_rafaga(t_pcb* pcb) {
	int alpha = lectura_de_config.HRRN_ALFA;
	pcb->estimado_prox_rafaga = alpha * pcb->tiempo_real_ejecucion + (1-alpha) * pcb->estimado_prox_rafaga; //pcb->estimado_prox_rafaga aca es como el estimado anterior
}

int calcular_R(t_pcb* pcb) {
	return (calcular_tiempo_en_ready(pcb->tiempo_llegada_ready) + pcb->estimado_prox_rafaga) / pcb->estimado_prox_rafaga;
}

int calcular_tiempo_en_ready(int segundos) {
	return time(NULL) - segundos;
}

t_pcb* obtener_proximo_a_ejecutar() {
	t_pcb* pcb;
	if(proximo_pcb_a_ejecutar_forzado != NULL) {
		t_pcb* proximo = proximo_pcb_a_ejecutar_forzado;
		proximo_pcb_a_ejecutar_forzado = NULL;
		return proximo;
	}
	else if(!strcmp(lectura_de_config.ALGORITMO_PLANIFICACION, "FIFO")) {
		return list_pop_con_mutex(ready_list, &mutex_ready_list);
	}
	else if (!strcmp(lectura_de_config.ALGORITMO_PLANIFICACION, "HRRN")) {
		pthread_mutex_lock(&mutex_ready_list);
		pcb = list_get_maximum(ready_list, (void*)calcular_R);
		list_remove_element(ready_list, pcb);
		pthread_mutex_unlock(&mutex_ready_list);
		return pcb;
	}
	else log_error(logger, "Error en la lectura del algoritmo de planificacion");
	exit(EXIT_FAILURE);
}

void manejar_io(t_args_io* args_io) {
	sleep(args_io->tiempo);
	ready_list_push(args_io->pcb); //Aca calcula el S (proxima rafaga), actualizo el tiempo_llegada_ready y hago sem_post(&sem_cant_ready)
	log_info(logger, "PID: %d - Estado Anterior: BLOCK - Estado Actual: READY", args_io->pcb->pid); //log obligatorio
	free(args_io);
}

void wait_recurso(t_pcb* pcb, char* nombre_recurso) {
	t_recurso* recurso = buscar_recurso(nombre_recurso);
	log_error(logger, "NOMBRE DE RECURSO: %s", recurso->nombre);
	if(recurso) {
		recurso->cantidad_disponibles--;
		log_info(logger, "PID: %d - Wait: %s - Instancias: %d", pcb->pid, nombre_recurso, recurso->cantidad_disponibles); //log obligatorio

		if(recurso->cantidad_disponibles < 0) { //Si el recurso no esta disponible
			pcb->tiempo_real_ejecucion = time(NULL) - pcb->tiempo_inicial_ejecucion;
			pcb->tiempo_inicial_ejecucion = 0; //Indica que no sigue en EXEC

			queue_push(recurso->cola_bloqueados, pcb); //Pasa a BLOCK
			log_info(logger, "PID: %d - Estado Anterior: EXEC - Estado Actual: BLOCK", pcb->pid); //log obligatorio
			log_info(logger, "PID: %d - Bloqueado por: %s", pcb->pid, nombre_recurso); //log obligatorio
		}
		else { //el recurso esta disponible, tiene que seguir ejecutando el mismo proceso
			log_warning(logger, "proceso_que_sigue_en_exec = %d", pcb->pid);
			proximo_pcb_a_ejecutar_forzado = pcb;
			sem_post(&sem_cant_ready); //Al no pasar por la funcion ready_list_push hay que hacerlo manualmente, vuelve a ejecutar sin pasar por ready
		}
	}
	else {
		log_error(logger, "El recurso %s no existe", nombre_recurso);
		exit_proceso(pcb); //no existe el recurso
	}
}

t_recurso* buscar_recurso(char* nombre_recurso) {
	t_recurso* recurso;
	for(int i = 0; i < list_size(list_recursos); i++) {
		recurso = list_get(list_recursos, i);
		if(!strcmp(recurso->nombre, nombre_recurso)) {
			return recurso;
		}
	}
	return NULL;
}

void signal_recurso(t_pcb* pcb, char* nombre_recurso) {
	t_recurso* recurso =  buscar_recurso(nombre_recurso);

	if(recurso) {
		recurso->cantidad_disponibles++;
		log_info(logger, "PID: %d - Signal: %s - Instancias: %d", pcb->pid, nombre_recurso, recurso->cantidad_disponibles); //log obligatorio

		if(recurso->cantidad_disponibles <= 0) { //Si habia al menos un proceso que estaba bloqueado
			t_pcb* pcb_a_desbloquear = queue_pop(recurso->cola_bloqueados);
			ready_list_push(pcb_a_desbloquear); //hace el sem_post(&sem_cant_ready)
			log_info(logger, "PID: %d - Estado Anterior: BLOCK - Estado Actual: READY", pcb_a_desbloquear->pid); //log obligatorio
		}

		log_warning(logger, "proceso_que_sigue_en_exec = %d", pcb->pid);
		proximo_pcb_a_ejecutar_forzado = pcb;
		sem_post(&sem_cant_ready); //Al no pasar por la funcion ready_list_push hay que hacerlo manualmente, vuelve a ejecutar sin pasar por ready
	}
	else {
		log_error(logger, "El recurso %s no existe", nombre_recurso);
		exit_proceso(pcb); //no existe el recurso
	}
}

void exit_proceso(t_pcb* pcb) {
	sem_post(&sem_multiprogramacion);
	enviar_fin_proceso(pcb->socket_consola, FINALIZACION_OK);

	log_info(logger, "PID: %d - Estado Anterior: EXEC - Estado Actual: EXIT", pcb->pid); //log obligatorio
	log_info(logger, "Finaliza el proceso %d - Motivo: SUCCESS", pcb->pid); //log obligatorio

	//avisarle a memoria para liberar la estructura
	liberar_pcb(pcb);
}

int list_remove_element(t_list *self, void *element) {
	int _is_the_element(void *data) {
		return element == data;
	}
	return list_remove_by_condition(self, (void*)_is_the_element) != NULL;
}

