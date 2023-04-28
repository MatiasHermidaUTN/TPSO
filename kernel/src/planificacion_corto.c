#include "../include/planificacion_corto.h"

int proceso_que_sigue_en_exec = -1; //Para ver si hay uno en WAIT o SIGNAL, que en realidad siguen en EXEC
//Si esta en -1, es que no hay ninguno, por eso lo inicializo asi

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
				ready_list_push(pcb_recibido); //Aca calcula el S (proxima rafaga), actualizo el tiempo_llegada_ready
				break;  					   //y hago sem_post(&sem_cant_ready) (solo necesito pasarle el pcb, porque ya se que es en Ready)

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

			case WAIT_EJECUTADO:
				char* recurso_a_usar = recibir_parametro_de_instruccion();
				pcb_recibido = recibir_pcb(socket_cpu);
				wait_recurso(pcb_recibido, recurso_a_usar);
				break;

			case SIGNAL_EJECUTADO:
				char* recurso_a_liberar = recibir_parametro_de_instruccion();
				pcb_recibido = recibir_pcb(socket_cpu);
				signal_recurso(pcb_recibido, recurso_a_liberar);
				break;

			case EXIT_EJECUTADO:
				pcb_recibido = recibir_pcb(socket_cpu);
				//log_info(logger, "AX: %s", pcb_recibido->registros_cpu.AX);
				exit_proceso(pcb_recibido); //Aca hace el sem_post(&sem_multiprogramacion)
				break;

			default:
				log_error(logger,"Error en la comunicacion entre el Kernel y la CPU");
				exit(EXIT_FAILURE);
		}
	}
}

void ready_list_push(t_pcb* pcb_recibido) { //Para ahorrar logica
	pcb_recibido->tiempo_llegada_ready = time(NULL); //Actualizo el momento en que llega a Ready

	if(proceso_que_sigue_en_exec == -1) { //Si pasa a Ready en serio, y no es uno que sigue en EXEC
		calcular_prox_rafaga(pcb_recibido); //S se calcula solo al que entra a Ready
	}

	list_push_con_mutex(ready_list, pcb_recibido, &mutex_ready_list);

	sem_post(&sem_cant_ready); //Dice que hay un nuevo proceso en Ready
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
	t_pcb* pcb;

	if(proceso_que_sigue_en_exec != -1) {
		for(int i = 0; i < list_size(ready_list); i++) {
			pcb = list_get(ready_list, i);
			if(pcb->pid == proceso_que_sigue_en_exec) {
				proceso_que_sigue_en_exec = -1; //Vuelve a decir que no hay uno que sigue en EXEC

				pthread_mutex_lock(&mutex_ready_list);
				list_remove_element(ready_list, pcb);
				pthread_mutex_unlock(&mutex_ready_list);
				return pcb;
			}
		}
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
	exit(2);
}

void manejar_io(t_args_io* args_io) {
	sleep(args_io->tiempo);
	ready_list_push(args_io->pcb); //Aca calcula el S (proxima rafaga), actualizo el tiempo_llegada_ready y hago sem_post(&sem_cant_ready)
	free(args_io);
}

void wait_recurso(t_pcb* pcb, char* recurso) {
	t_recurso* recurso_a_buscar;
	t_msj_kernel_cpu continuar_exec;

	if(existe_recurso(recurso)) {
		recurso_a_buscar = buscar_recurso(recurso); //Podria hacerse con list_find?
		recurso_a_buscar->cantidad_disponibles--;

		if(recurso_a_buscar->cantidad_disponibles < 0) { //Si el recurso no esta disponible
			pcb->tiempo_real_ejecucion = time(NULL) - pcb->tiempo_inicial_ejecucion;
			//Lo hago aca pues no se hace en el cpu
			pcb->tiempo_inicial_ejecucion = 0; //Indica que no sigue en EXEC

			queue_push(recurso_a_buscar->cola_bloqueados, pcb); //Pasa a BLOCK
			log_info(logger, "PID: %d - Estado Anterior: EXEC - Estado Actual: BLOCK", pcb->pid); //log obligatorio
			log_info(logger, "PID: %d - Bloqueado por: %s", pcb->pid, recurso); //log obligatorio
		}
		else {
			proceso_que_sigue_en_exec = pcb->pid;
			ready_list_push(pcb); //hacer el sem_post(&sem_cant_ready)
		}
	}
	else {
		exit_proceso(pcb);
	}
}

int existe_recurso(char* nombre_recurso) {
	t_recurso* recurso;
	for(int i = 0; i < list_size(list_recursos); i++) {
		recurso = list_get(list_recursos, i);
		if(!strcmp(recurso->nombre, nombre_recurso)) {
			return 1;
		}
	}
	return 0;
}
// Logica repetida! que asco!! (pero ya fue)
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

void signal_recurso(t_pcb* pcb, char* recurso) {
	t_recurso* recurso_a_buscar;
	t_pcb* pcb_a_desbloquear;

	if(existe_recurso(recurso)) {
		recurso_a_buscar = buscar_recurso(recurso);
		recurso_a_buscar->cantidad_disponibles++;

		if(recurso_a_buscar->cantidad_disponibles <= 0) { //Si habia al menos un proceso que estaba bloqueado
			pcb_a_desbloquear = queue_pop(recurso_a_buscar->cola_bloqueados);
			ready_list_push(pcb_a_desbloquear); //hace el sem_post(&sem_cant_ready)

			proceso_que_sigue_en_exec = pcb->pid;
			ready_list_push(pcb); //hacer el sem_post(&sem_cant_ready)
		}
	}
	else {
		exit_proceso(pcb);
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

