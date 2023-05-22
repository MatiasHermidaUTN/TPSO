#include "../include/planificacion_corto.h"

void planificar_corto() {
	t_pcb* pcb;
	t_pcb* pcb_recibido;
	char** parametros;

	while(1) {
		sem_wait(&sem_cant_ready); //entra solo si hay algun proceso en Ready, es una espera no activa
		pcb = obtener_proximo_a_ejecutar();

	    //Comienza ejecucion
		char** sin_parametros = string_array_new(); //TODO: hardcodeado nashe
		enviar_pcb(socket_cpu, pcb, PCB_A_EJECUTAR, sin_parametros); //NULL porque no se le pasa ningun parametro extra
		string_array_destroy(sin_parametros);
		liberar_pcb(pcb);

		t_msj_kernel_cpu respuesta = esperar_cpu();

		switch(respuesta) {
			case IO_EJECUTADO:
				parametros = recibir_parametros_de_instruccion();
				char* tiempo_como_string = strdup(parametros[0]);
				int tiempo_a_bloquear = atoi(tiempo_como_string);
				string_array_destroy(parametros);
				free(tiempo_como_string);

				pcb_recibido = recibir_pcb(socket_cpu);
				pcb_recibido->tiempo_real_ejecucion = time(NULL) - pcb_recibido->tiempo_inicial_ejecucion;
				//Finaliza ejecucion

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

			case F_OPEN_EJECUTADO:
				parametros = recibir_parametros_de_instruccion();
				log_warning(logger, "parametros[0]: %s", parametros[0]);
				pcb_recibido = recibir_pcb(socket_cpu);

				t_recurso* archivo_a_abrir = buscar_recurso(parametros[0], list_archivos);

				t_archivo_abierto* archivo_abierto = malloc(sizeof(t_archivo_abierto));
				archivo_abierto->nombre_archivo = strdup(parametros[0]);
				archivo_abierto->posicion_actual = 0;
				list_add(pcb_recibido->archivos_abiertos,archivo_abierto);

				if(archivo_a_abrir){//el archivo esta abierto

					archivo_a_abrir->cantidad_disponibles--;
					queue_push(archivo_a_abrir->cola_bloqueados,pcb_recibido);



				}else{ //el archivo no esta abierto
					enviar_msj(EXISTE_ARCHIVO,socket_fileSystem);
					int msj_recibido = recibir_msj(socket_fileSystem);

					t_recurso* archivo = malloc(sizeof(t_recurso));
					archivo->nombre = strdup(parametros[0]);
					archivo->cantidad_disponibles = 0;
					list_add(list_archivos,archivo);

					if(msj_recibido == EL_ARCHIVO_NO_EXISTE){
						char** parametros_a_enviar = string_array_new();
						string_array_push(&parametros_a_enviar,parametros[0]);
						enviar_msj_con_parametros(CREAR_ARCHIVO,parametros_a_enviar,socket_fileSystem);

						int rta = recibir_msj(socket_fileSystem);
						if(rta == EL_ARCHIVO_FUE_CREADO){
							log_info(logger,"Se abrio el archivo"); //en realidad nunca va a fallar. esto tendria que haberse hecho con un solo mensaje pero lo separaron en abrir y crear.
						} else{
							log_error(logger,"El filesystem no pudo crear el archivo");
							exit(EXIT_FAILURE);
						}


						string_array_destroy(parametros_a_enviar);

					} else {
						log_error(logger,"Error en la comunicacion entre el kernel y el file system");
						exit(EXIT_FAILURE);
					}

					proximo_pcb_a_ejecutar_forzado = pcb_recibido;
					sem_post(&sem_cant_ready);

				}

				string_array_destroy(parametros);
				break;

			case F_CLOSE_EJECUTADO:
				parametros = recibir_parametros_de_instruccion();
				//TODO: AVISAR A FILESYSTEM

				string_array_destroy(parametros);
				break;

			case F_SEEK_EJECUTADO:
				parametros = recibir_parametros_de_instruccion();
				//log_warning(logger, "parametros[0]: %s", parametros[0]);
				//log_warning(logger, "parametros[1]: %s", parametros[1]);
				//TODO: AVISAR A FILESYSTEM

				string_array_destroy(parametros);

				pcb_recibido = recibir_pcb(socket_cpu);
				proximo_pcb_a_ejecutar_forzado = pcb_recibido;
				sem_post(&sem_cant_ready); //Al no pasar por la funcion ready_list_push hay que hacerlo manualmente, vuelve a ejecutar sin pasar por ready
				break;

			case F_READ_EJECUTADO:
				parametros = recibir_parametros_de_instruccion();
				//TODO: AVISAR A FILESYSTEM

				string_array_destroy(parametros);
				break;

			case F_WRITE_EJECUTADO:
				parametros = recibir_parametros_de_instruccion();
				//TODO: AVISAR A FILESYSTEM

				//log_warning(logger, "parametros[0]: %s", parametros[0]);
				//log_warning(logger, "parametros[1]: %s", parametros[1]);
				//log_warning(logger, "parametros[2]: %s", parametros[2]);
				//TODO: AVISAR A FILESYSTEM

				string_array_destroy(parametros);

				pcb_recibido = recibir_pcb(socket_cpu);
				proximo_pcb_a_ejecutar_forzado = pcb_recibido;
				sem_post(&sem_cant_ready); //Al no pasar por la funcion ready_list_push hay que hacerlo manualmente, vuelve a ejecutar sin pasar por ready
				break;

			case F_TRUNCATE_EJECUTADO:
				parametros = recibir_parametros_de_instruccion();
				//TODO: AVISAR A FILESYSTEM

				string_array_destroy(parametros);
				break;

			case WAIT_EJECUTADO:
				parametros = recibir_parametros_de_instruccion();
				//char* nombre_recurso_wait = string_array_pop(parametros); //TODO: fijarse si anda
				char* nombre_recurso_wait = strdup(parametros[0]);
				string_array_destroy(parametros);

				pcb_recibido = recibir_pcb(socket_cpu);
				wait_recurso(pcb_recibido, nombre_recurso_wait);
				free(nombre_recurso_wait);
				break;

			case SIGNAL_EJECUTADO:
				parametros = recibir_parametros_de_instruccion();
				//char* nombre_recurso_signal = string_array_pop(parametros); //TODO: fijarse si anda
				char* nombre_recurso_signal = strdup(parametros[0]);
				string_array_destroy(parametros);

				pcb_recibido = recibir_pcb(socket_cpu);
				signal_recurso(pcb_recibido, nombre_recurso_signal);
				free(nombre_recurso_signal);
				break;

			case CREATE_SEGMENT_EJECUTADO:
				parametros = recibir_parametros_de_instruccion();
				pcb_recibido = recibir_pcb(socket_cpu);

				//TODO: AVISAR A MEMORIA
				string_array_destroy(parametros);

				proximo_pcb_a_ejecutar_forzado = pcb_recibido;
				sem_post(&sem_cant_ready); //Al no pasar por la funcion ready_list_push hay que hacerlo manualmente, vuelve a ejecutar sin pasar por ready

				break;

			case DELETE_SEGMENT_EJECUTADO:
				parametros = recibir_parametros_de_instruccion();
				pcb_recibido = recibir_pcb(socket_cpu);

				//TODO: AVISAR A MEMORIA
				string_array_destroy(parametros);

				proximo_pcb_a_ejecutar_forzado = pcb_recibido;
				sem_post(&sem_cant_ready); //Al no pasar por la funcion ready_list_push hay que hacerlo manualmente, vuelve a ejecutar sin pasar por ready

				break;

			case YIELD_EJECUTADO: //vuelve a ready
				pcb_recibido = recibir_pcb(socket_cpu);
				pcb_recibido->tiempo_real_ejecucion = time(NULL) - pcb_recibido->tiempo_inicial_ejecucion;
				//Finaliza ejecucion

				log_info(logger, "PID: %d - Estado Anterior: EXEC - Estado Actual: READY", pcb_recibido->pid); //log obligatorio
				ready_list_push(pcb_recibido); //Aca calcula el S (proxima rafaga), actualizo el tiempo_llegada_ready y hago sem_post(&sem_cant_ready) (solo necesito pasarle el pcb, porque ya se que es en Ready)
				break;

			case EXIT_EJECUTADO:
				pcb_recibido = recibir_pcb(socket_cpu);
				exit_proceso(pcb_recibido); //Aca hace el sem_post(&sem_multiprogramacion)
				break;

			default:
				log_error(logger, "Error en la comunicacion entre el Kernel y la CPU");
				exit(EXIT_FAILURE);
		}
	}
}

void ready_list_push(t_pcb* pcb_recibido) {
	pcb_recibido->tiempo_llegada_ready = time(NULL); //Actualizo el momento en que llega a Ready
	calcular_prox_rafaga(pcb_recibido); //S se calcula solo al que entra a Ready

	list_push_con_mutex(ready_list, pcb_recibido, &mutex_ready_list);
	log_pids(); //log obligatorio

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
	if(proximo_pcb_a_ejecutar_forzado) { //No hay que resetear el tiempo_inicial_ejecucion porque sigue ejecutando el mismo proceso
		t_pcb* proximo = proximo_pcb_a_ejecutar_forzado;
		proximo_pcb_a_ejecutar_forzado = NULL;
		return proximo;
	}
	else if(!strcmp(lectura_de_config.ALGORITMO_PLANIFICACION, "FIFO")) {
		pcb = list_pop_con_mutex(ready_list, &mutex_ready_list);
		pcb->tiempo_inicial_ejecucion = time(NULL);
		log_info(logger, "PID: %d - Estado Anterior: READY - Estado Actual: EXEC", pcb->pid);
		return pcb;

	}
	else if (!strcmp(lectura_de_config.ALGORITMO_PLANIFICACION, "HRRN")) {
		pthread_mutex_lock(&mutex_ready_list);

		//TODO: me hace ruido que tengamos que usar nuestras propias funciones
		//pcb = list_get_maximum(ready_list, (void*)calcular_R);
		pcb = list_get_max_R(ready_list);

		list_remove_pcb(ready_list, pcb); //lo remueve pero no lo libera
		pthread_mutex_unlock(&mutex_ready_list);
		pcb->tiempo_inicial_ejecucion = time(NULL);
		log_info(logger, "PID: %d - Estado Anterior: READY - Estado Actual: EXEC", pcb->pid);
		return pcb;
	}
	else log_error(logger, "Error en la lectura del algoritmo de planificacion");
	exit(EXIT_FAILURE);
}

void manejar_io(t_args_io* args_io) {
	sleep(args_io->tiempo);
	log_info(logger, "PID: %d - Estado Anterior: BLOCK - Estado Actual: READY", args_io->pcb->pid); //log obligatorio
	ready_list_push(args_io->pcb); //Aca calcula el S (proxima rafaga), actualizo el tiempo_llegada_ready y hago sem_post(&sem_cant_ready)
	free(args_io);
}

void wait_recurso(t_pcb* pcb, char* nombre_recurso) {
	t_recurso* recurso = buscar_recurso(nombre_recurso,list_recursos);
	if(recurso) {
		recurso->cantidad_disponibles--;
		log_info(logger, "PID: %d - Wait: %s - Instancias: %d", pcb->pid, nombre_recurso, recurso->cantidad_disponibles); //log obligatorio

		if(recurso->cantidad_disponibles < 0) { //Si el recurso no esta disponible
			pcb->tiempo_real_ejecucion = time(NULL) - pcb->tiempo_inicial_ejecucion;

			queue_push(recurso->cola_bloqueados, pcb); //Pasa a BLOCK
			log_info(logger, "PID: %d - Estado Anterior: EXEC - Estado Actual: BLOCK", pcb->pid); //log obligatorio
			log_info(logger, "PID: %d - Bloqueado por: %s", pcb->pid, nombre_recurso); //log obligatorio
		}
		else { //el recurso esta disponible, tiene que seguir ejecutando el mismo proceso
			proximo_pcb_a_ejecutar_forzado = pcb;
			sem_post(&sem_cant_ready); //Al no pasar por la funcion ready_list_push hay que hacerlo manualmente, vuelve a ejecutar sin pasar por ready
		}
	}
	else {
		log_error(logger, "El recurso %s no existe", nombre_recurso);
		exit_proceso(pcb); //no existe el recurso
	}
}

t_recurso* buscar_recurso(char* nombre_recurso, t_list* lista) {
	t_recurso* recurso;
	for(int i = 0; i < list_size(lista); i++) {
		recurso = list_get(lista, i);
		if(!strcmp(recurso->nombre, nombre_recurso)) {
			return recurso;
		}
	}
	return NULL;
}

void signal_recurso(t_pcb* pcb, char* nombre_recurso) {
	t_recurso* recurso = buscar_recurso(nombre_recurso,list_recursos);

	if(recurso) {
		recurso->cantidad_disponibles++;
		log_info(logger, "PID: %d - Signal: %s - Instancias: %d", pcb->pid, nombre_recurso, recurso->cantidad_disponibles); //log obligatorio

		if(recurso->cantidad_disponibles <= 0) { //Si habia al menos un proceso que estaba bloqueado
			t_pcb* pcb_a_desbloquear = queue_pop(recurso->cola_bloqueados);
			log_info(logger, "PID: %d - Estado Anterior: BLOCK - Estado Actual: READY", pcb_a_desbloquear->pid); //log obligatorio
			ready_list_push(pcb_a_desbloquear); //hace el sem_post(&sem_cant_ready)
		}

		proximo_pcb_a_ejecutar_forzado = pcb;
		sem_post(&sem_cant_ready); //Al no pasar por la funcion ready_list_push hay que hacerlo manualmente, vuelve a ejecutar sin pasar por ready
	}
	else {
		log_error(logger, "El recurso %s no existe", nombre_recurso);
		exit_proceso(pcb); //no existe el recurso
	}
}

void exit_proceso(t_pcb* pcb) {
	enviar_fin_proceso(pcb->socket_consola, FINALIZACION_OK);

	log_info(logger, "PID: %d - Estado Anterior: EXEC - Estado Actual: EXIT", pcb->pid); //log obligatorio
	log_info(logger, "Finaliza el proceso %d - Motivo: SUCCESS", pcb->pid); //log obligatorio

	sem_post(&sem_multiprogramacion);

	//TODO: avisarle a memoria para liberar la estructura
	liberar_pcb(pcb);
}

void list_remove_pcb(t_list *lista, t_pcb *pcb) {
	t_pcb* elemento;
	for(int i = 0; i < list_size(lista); i++) {
		elemento = list_get(lista, i);
		if(elemento->pid == pcb->pid) {
			elemento = list_remove(lista, i);
		}
	}
}

t_pcb* list_get_max_R(t_list* lista) {
	t_pcb* pcb_max = list_get(lista, 0);
	t_pcb* pcb;
	for(int i = 0; i < list_size(lista); i++) {
		pcb = list_get(lista, i);
		if(calcular_R(pcb) > calcular_R(pcb_max)) {
			pcb_max = pcb;
		}
	}
	return pcb_max;
}

