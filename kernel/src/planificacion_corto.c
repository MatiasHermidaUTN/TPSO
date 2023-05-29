#include "../include/planificacion_corto.h"

void planificar_corto() {
	t_pcb* pcb;
	t_pcb* pcb_recibido;
	char** parametros;

	while(1) {
		sem_wait(&sem_cant_ready); //entra solo si hay algun proceso en Ready, es una espera no activa
		pcb = obtener_proximo_a_ejecutar();

	    //Comienza ejecucion
		char** sin_parametros = string_array_new(); //hardcodeado nashe
		enviar_pcb(socket_cpu, pcb, PCB_A_EJECUTAR, sin_parametros);
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
				pcb_recibido = recibir_pcb(socket_cpu);

				t_recurso* archivo_a_abrir = buscar_recurso(parametros[0], list_archivos);

				t_archivo_abierto* archivo_abierto = malloc(sizeof(t_archivo_abierto));
				archivo_abierto->nombre_archivo = strdup(parametros[0]);
				archivo_abierto->posicion_actual = 0;
				list_add(pcb_recibido->archivos_abiertos, archivo_abierto);

				if(archivo_a_abrir) { //si el archivo esta abierto
					bloquear_pcb_por_archivo(pcb_recibido, archivo_a_abrir->nombre);

				}
				else { //si no esta abierto
					enviar_msj_con_parametros(socket_fileSystem, EXISTE_ARCHIVO, parametros);
					sem_wait(&sem_respuesta_fs);

					log_info(logger, "PID: %d - Abrir Archivo: %s", pcb_recibido->pid, archivo_abierto->nombre_archivo); //log obligatorio

					t_recurso* archivo = malloc(sizeof(t_recurso));
					archivo->nombre = strdup(parametros[0]);
					archivo->cantidad_disponibles = 0;
					archivo->cola_bloqueados = queue_create();
					pthread_mutex_init(&(archivo->mutex_archivo), NULL);
					list_add(list_archivos, archivo);

					if(respuesta_fs_global == EL_ARCHIVO_NO_EXISTE) {
						enviar_msj_con_parametros(socket_fileSystem, CREAR_ARCHIVO, parametros);
						sem_wait(&sem_respuesta_fs);

						if(respuesta_fs_global == EL_ARCHIVO_FUE_CREADO) {
							log_warning(logger, "Se abrio el archivo"); //en realidad nunca va a fallar. Esto tendria que haberse hecho con un solo mensaje pero lo separaron en abrir y crear.
						}
						else {
							log_error(logger, "El File System no pudo crear el archivo");
							exit(EXIT_FAILURE);
						}

					}
					else { // EL_ARCHIVO_YA_EXISTE
						log_warning(logger, "Se abrio un archivo que ya existia en el File System");
					}

					mantener_pcb_en_exec(pcb_recibido);
				}

				string_array_destroy(parametros);
				break;

			case F_CLOSE_EJECUTADO:
				parametros = recibir_parametros_de_instruccion();
				pcb_recibido = recibir_pcb(socket_cpu);

				cerrar_archivo(pcb_recibido, parametros[0]);

				mantener_pcb_en_exec(pcb_recibido);

				string_array_destroy(parametros);
				break;

			case F_SEEK_EJECUTADO:
				parametros = recibir_parametros_de_instruccion();
				pcb_recibido = recibir_pcb(socket_cpu);
				t_archivo_abierto* archivo_a_modificar = buscar_archivo_en_pcb(pcb_recibido, parametros[0]);
				archivo_a_modificar->posicion_actual = atoi(parametros[1]);

				log_info(logger, "PID: %d - Actualizar puntero Archivo: %s - Puntero %d", pcb_recibido->pid, archivo_a_modificar->nombre_archivo, archivo_a_modificar->posicion_actual); //log obligatorio

				string_array_destroy(parametros);

				mantener_pcb_en_exec(pcb_recibido);
				break;

			case F_READ_EJECUTADO: case F_WRITE_EJECUTADO:
				/*
				F_READ lee del archivo y escribe en memoria
				F_WRITE lee de memoria y escribe en archivo
				*/
				//Me fijo si es F_READ o F_WRITE (hice esto para evitar mucha repetición de lógica)
				char* accion;
				t_msj_kernel_fileSystem mensaje_a_mandar;
				if(respuesta == F_READ_EJECUTADO) {
					mensaje_a_mandar = LEER_ARCHIVO;
					accion = strdup("Leer");
				}
				else { //F_WRITE
					mensaje_a_mandar = ESCRIBIR_ARCHIVO;
					accion = strdup("Escribir");
				}

				pthread_mutex_lock(&mutex_cantidad_de_reads_writes);
				if(!cantidad_de_reads_writes) {
					sem_wait(&sem_compactacion); //solo avisa si habían 0 operaciones
				} //Solo sirve para que luego CREATE_SEGMENT tenga que hacer un wait y, si habían operaciones, se quede esperando a que finalicen
				cantidad_de_reads_writes++;
				pthread_mutex_unlock(&mutex_cantidad_de_reads_writes);


				parametros = recibir_parametros_de_instruccion();
				pcb_recibido = recibir_pcb(socket_cpu);

				t_archivo_abierto* archivo = buscar_archivo_en_pcb(pcb_recibido, parametros[0]);

				string_array_push(&parametros, string_itoa(archivo->posicion_actual));
				string_array_push(&parametros, string_itoa(pcb_recibido->pid));
				bloquear_pcb_por_archivo(pcb_recibido, parametros[0]);
				enviar_msj_con_parametros(socket_fileSystem, mensaje_a_mandar, parametros);

				log_info(logger, "PID: %d - %s Archivo: %s - Puntero %d - Dirección Memoria %s - Tamaño %s", pcb_recibido->pid, accion, archivo->nombre_archivo, archivo->posicion_actual, parametros[1], parametros[2]); //log obligatorio
				//TODO: el log (y el puntero) tiene que ser antes o después de haber leido?
				//TODO: dirección de memoria es lo mismo que dirección física?

				free(accion);
				string_array_destroy(parametros);
				break;

			case F_TRUNCATE_EJECUTADO:
				parametros = recibir_parametros_de_instruccion();
				pcb_recibido = recibir_pcb(socket_cpu);
				char* pid_a_enviar = string_itoa(pcb_recibido->pid);

				string_array_push(&parametros, pid_a_enviar);
				bloquear_pcb_por_archivo(pcb_recibido, parametros[0]);
				enviar_msj_con_parametros(socket_fileSystem, TRUNCAR_ARCHIVO, parametros);

				log_info(logger, "PID: %d - Archivo: %s - Tamaño: %s", pcb_recibido->pid, parametros[0], parametros[1]); //log obligatorio

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

				//TODO: testear cuando esté memoria lista
				crear_segmento(pcb_recibido, parametros);

				string_array_destroy(parametros);

				break;

			case DELETE_SEGMENT_EJECUTADO:
				parametros = recibir_parametros_de_instruccion();
				pcb_recibido = recibir_pcb(socket_cpu);

				//TODO: testear cuando esté memoria lista

				enviar_msj_con_parametros(socket_memoria, ELIMINAR_SEGMENTO, parametros); //Es un solo parametro, que es el id

				int id_segmento_a_eliminar = atoi(parametros[0]);
				eliminar_segmento(pcb_recibido, id_segmento_a_eliminar);
				log_info(logger, "PID: %d - Eliminar Segmento - Id Segmento: %d", pcb_recibido->pid, id_segmento_a_eliminar); //log obligatorio
				//TODO: fijarse si hace falta que reciba realmente algo de memoria

				string_array_destroy(parametros);

				mantener_pcb_en_exec(pcb_recibido);

				break;

			case YIELD_EJECUTADO: //vuelve a ready
				pcb_recibido = recibir_pcb(socket_cpu);
				pcb_recibido->tiempo_real_ejecucion = time(NULL) - pcb_recibido->tiempo_inicial_ejecucion;
				//Finaliza ejecucion

				ready_list_push(pcb_recibido); //Aca calcula el S (proxima rafaga), actualizo el tiempo_llegada_ready y hago sem_post(&sem_cant_ready) (solo necesito pasarle el pcb, porque ya sé que es en Ready)
				break;

			case EXIT_EJECUTADO:
				pcb_recibido = recibir_pcb(socket_cpu);
				exit_proceso(pcb_recibido, SUCCESS); //Aca hace el sem_post(&sem_multiprogramacion)
				break;

			case EXIT_CON_SEG_FAULT_EJECUTADO:
				parametros = recibir_parametros_de_instruccion();
				pcb_recibido = recibir_pcb(socket_cpu);

				cerrar_archivo(pcb_recibido, parametros[0]);
				//TODO: fijarse si realmente tiene que cerrar el archivo, o matar al kernel o que el archivo se quede abierto para siempre

				exit_proceso(pcb_recibido, SEG_FAULT); //Aca hace el sem_post(&sem_multiprogramacion)

				string_array_destroy(parametros);
				break;

			default:
				log_error(logger, "Error en la comunicacion entre el Kernel y la CPU");
				exit(EXIT_FAILURE);
		}
	}
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
		log_info(logger, "PID: %d - Estado Anterior: READY - Estado Actual: EXEC", pcb->pid); //log obligatorio
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
		log_info(logger, "PID: %d - Estado Anterior: READY - Estado Actual: EXEC", pcb->pid); //log obligatorio
		return pcb;
	}
	else log_error(logger, "Error en la lectura del algoritmo de planificacion");
	exit(EXIT_FAILURE);
}

void manejar_io(t_args_io* args_io) {
	sleep(args_io->tiempo);
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
			mantener_pcb_en_exec(pcb);
		}
	}
	else {
		log_error(logger, "El recurso %s no existe", nombre_recurso);
		exit_proceso(pcb, SUCCESS); //no existe el recurso
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
	t_recurso* recurso = buscar_recurso(nombre_recurso, list_recursos);

	if(recurso) {
		recurso->cantidad_disponibles++;
		log_info(logger, "PID: %d - Signal: %s - Instancias: %d", pcb->pid, nombre_recurso, recurso->cantidad_disponibles); //log obligatorio

		if(recurso->cantidad_disponibles <= 0) { //Si habia al menos un proceso que estaba bloqueado
			t_pcb* pcb_a_desbloquear = queue_pop(recurso->cola_bloqueados);
			ready_list_push(pcb_a_desbloquear); //hace el sem_post(&sem_cant_ready)
		}

		mantener_pcb_en_exec(pcb);
	}
	else {
		log_error(logger, "El recurso %s no existe", nombre_recurso);
		exit_proceso(pcb, SUCCESS); //no existe el recurso
	}
}

void exit_proceso(t_pcb* pcb, t_msj_kernel_consola mensaje) {
	enviar_fin_proceso(pcb->socket_consola, mensaje);

	log_info(logger, "PID: %d - Estado Anterior: EXEC - Estado Actual: EXIT", pcb->pid); //log obligatorio
	log_info(logger, "Finaliza el proceso %d - Motivo: %s", pcb->pid, mensaje_de_finalizacion_a_string(mensaje)); //log obligatorio

	sem_post(&sem_multiprogramacion);

	//TODO: avisarle a memoria para liberar la estructura
	liberar_pcb(pcb);
}

char* mensaje_de_finalizacion_a_string(t_msj_kernel_consola mensaje) {
	switch(mensaje) {
		case SUCCESS:
			return "SUCCESS";
		case OUT_OF_MEMORY:
			return "OUT_OF_MEMORY";
		case SEG_FAULT:
			return "SEG_FAULT";
		default:
			log_error(logger, "Error en el envio de mensaje de finalizacion");
			exit(EXIT_FAILURE);
	}
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

void cerrar_archivo(t_pcb* pcb_recibido, char* nombre_archivo) {
	eliminar_archivo(pcb_recibido, nombre_archivo);

	t_recurso* archivo_a_cerrar = buscar_recurso(nombre_archivo, list_archivos);
	archivo_a_cerrar->cantidad_disponibles++;

	log_info(logger, "PID: %d - Cerrar Archivo: %s", pcb_recibido->pid, archivo_a_cerrar->nombre); //log obligatorio

	if(queue_is_empty(archivo_a_cerrar->cola_bloqueados)) {
		// Elimina al archivo de la lista global de archivos abiertos
		list_remove_recurso(list_archivos, archivo_a_cerrar);

		// Elimina toda referencia al archivo
		queue_destroy(archivo_a_cerrar->cola_bloqueados);
		free(archivo_a_cerrar->nombre);
		free(archivo_a_cerrar);
	}
	else {
		t_pcb* pcb_a_desbloquear = queue_pop_con_mutex(archivo_a_cerrar->cola_bloqueados, &(archivo_a_cerrar->mutex_archivo));

		ready_list_push(pcb_a_desbloquear);

		log_info(logger, "PID: %d - Abrir Archivo: %s", pcb_a_desbloquear->pid, archivo_a_cerrar->nombre); //log obligatorio
		//TODO: me suena raro que tenga que poner que abrio el archivo estando en Ready
	}
}

void eliminar_archivo(t_pcb *pcb, char* nombre) {
	t_archivo_abierto* archivo_a_eliminar;
	for(int i = 0; i < list_size(pcb->archivos_abiertos); i++) {
		archivo_a_eliminar = list_get(pcb->archivos_abiertos, i);
		if(!strcmp(archivo_a_eliminar->nombre_archivo, nombre)) {
			list_remove(pcb->archivos_abiertos, i);
			destruir_archivo_abierto(archivo_a_eliminar);
		}
	}
}

void list_remove_recurso(t_list *lista, t_recurso *recurso) {
	t_recurso* elemento;
	for(int i = 0; i < list_size(lista); i++) {
		elemento = list_get(lista, i);
		if(!strcmp(elemento->nombre, recurso->nombre)){
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

t_archivo_abierto* buscar_archivo_en_pcb(t_pcb* pcb, char* nombre) {
	t_archivo_abierto* elemento;
	for(int i = 0; i < list_size(pcb->archivos_abiertos); i++) {
		elemento = list_get(pcb->archivos_abiertos, i);
		if(!strcmp(elemento->nombre_archivo, nombre)) {
			elemento = list_get(pcb->archivos_abiertos, i);
		}
	}
	return elemento;
}

void bloquear_pcb_por_archivo(t_pcb* pcb, char* nombre_archivo) {
	pcb->tiempo_real_ejecucion = time(NULL) - pcb->tiempo_inicial_ejecucion;

	t_recurso* archivo = buscar_recurso(nombre_archivo, list_archivos);

	queue_push_con_mutex(archivo->cola_bloqueados, pcb, &(archivo->mutex_archivo));
	archivo->cantidad_disponibles--;

	log_info(logger, "PID: %d - Estado Anterior: EXEC - Estado Actual: BLOCK", pcb->pid); //log obligatorio
	log_info(logger, "PID: %d - Bloqueado por: %s", pcb->pid, nombre_archivo); //log obligatorio
}

void crear_segmento(t_pcb* pcb_recibido, char** parametros) {
	enviar_msj_con_parametros(socket_memoria, CREAR_SEGMENTO, parametros); //Solo importa enviar parametros[1], que es el tamanio

	t_msj_kernel_memoria mensaje_recibido = recibir_msj(socket_memoria);

	switch(mensaje_recibido) {
		case SEGMENTO_CREADO:
			char** parametros_recibidos_de_memoria = recibir_parametros_de_mensaje(socket_memoria);

			t_segmento* segmento_a_crear = malloc(sizeof(t_segmento)); //Se libera en DELETE_SEGMENT en eliminar_segmento()
			segmento_a_crear->id = atoi(parametros[0]);
			segmento_a_crear->direccion_base = atoi(parametros_recibidos_de_memoria[0]);
			segmento_a_crear->tamanio = atoi(parametros[1]);

			list_add(pcb_recibido->tabla_segmentos, segmento_a_crear);
			log_info(logger, "PID: %d - Crear Segmento - Id: %d - Tamaño: %d", pcb_recibido->pid, segmento_a_crear->id, segmento_a_crear->tamanio); //log obligatorio

			string_array_destroy(parametros_recibidos_de_memoria);

			mantener_pcb_en_exec(pcb_recibido);
			break;

		case NO_HAY_ESPACIO_DISPONIBLE:
			exit_proceso(pcb_recibido, OUT_OF_MEMORY);
			break;

		case HAY_QUE_COMPACTAR:

			log_info(logger, "Compactación: Esperando Fin de Operaciones de FS"); //log obligatorio
			sem_wait(&sem_compactacion);

			log_info(logger, "Compactación: Se solicitó compactación"); //log obligatorio

			enviar_msj(socket_memoria, COMPACTAR);

			if(recibir_msj(socket_memoria) == MEMORIA_COMPACTADA) {
				log_info(logger, "Se finalizó el proceso de compactación"); //log obligatorio
				sem_post(&sem_compactacion);

				actualizar_segmentos(pcb_recibido);

				crear_segmento(pcb_recibido, parametros);
			}
			else {
				log_error(logger, "Error en la compactacion");
				exit(EXIT_FAILURE);
			}

			break;

		default:
			log_error(logger, "Error en el recibo de mensaje de memoria");
			exit(EXIT_FAILURE);
	}
}

void eliminar_segmento(t_pcb* pcb, int id) {
	t_segmento* segmento_a_eliminar;
	for(int i = 0; i < list_size(pcb->tabla_segmentos); i++) {
		segmento_a_eliminar = list_get(pcb->tabla_segmentos, i);
		if(segmento_a_eliminar->id == id) {
			list_remove(pcb->tabla_segmentos, i);
			free(segmento_a_eliminar);
		}
	}
}

void actualizar_segmentos(t_pcb* pcb_en_exec) {
	 //TODO: recibe una lista de procesos (id + tabla_segmentos)

	t_list* procesos = recibir_procesos_con_segmentos(socket_memoria);

	t_proceso_actualizado* proceso; //es distinto a t_pcb, pues solo tiene id + tabla_segmentos

	proceso = list_remove_if_pid_equals_to(procesos, pcb_en_exec->pid);
	actualizar_segmentos_de_pcb(pcb_en_exec, proceso->tabla_segmentos);

	actualizar_segmentos_de_lista(ready_list, procesos);

	t_recurso* recurso;
	for(int i = 0; i < list_size(list_recursos); i++) {
		recurso = list_get(list_recursos, i);
		actualizar_segmentos_de_cola(recurso->cola_bloqueados, procesos);
	}
	for(int i = 0; i < list_size(list_archivos); i++) {
		recurso = list_get(list_archivos, i);
		actualizar_segmentos_de_cola(recurso->cola_bloqueados, procesos);
	}
}

void actualizar_segmentos_de_lista(t_list* lista, t_list* procesos) {
	t_pcb* pcb;

	t_proceso_actualizado* proceso; //es distinto a t_pcb, pues solo tiene id + tabla_segmentos

	for(int i = 0; i < list_size(lista); i++) {
		pcb = list_remove(lista, 0);

		proceso = list_remove_if_pid_equals_to(procesos, pcb->pid);

		actualizar_segmentos_de_pcb(pcb, proceso->tabla_segmentos);
		list_add(lista, pcb);
	}
}

void actualizar_segmentos_de_cola(t_queue* cola, t_list* procesos) {
	t_pcb* pcb;

	t_proceso_actualizado* proceso; //es distinto a t_pcb, pues solo tiene id + tabla_segmentos

	for(int i = 0; i < queue_size(cola); i++) {
		pcb = queue_pop(cola);

		proceso = list_remove_if_pid_equals_to(procesos, pcb->pid);

		actualizar_segmentos_de_pcb(pcb, proceso->tabla_segmentos);
		queue_push(cola, pcb);
	}
}

void actualizar_segmentos_de_pcb(t_pcb* pcb, t_list* segmentos) {
	for(int i = 0; i < list_size(pcb->tabla_segmentos); i++) {
		list_remove(pcb->tabla_segmentos, 0); //Elimina el primero
		list_add(pcb->tabla_segmentos, list_remove(segmentos, 0)); //Lo agrega al final
		//No me importa el orden
	}
}

t_proceso_actualizado* list_remove_if_pid_equals_to(t_list* procesos, int pid) {
	t_proceso_actualizado* proceso;
	for(int i = 0; i < list_size(procesos); i++) {
		proceso = list_get(procesos, i);
		if(proceso->pid == pid) {
			return list_remove(procesos, i);
		}
	}

	log_error(logger, "Error al obtener pid de proceso con segmentos actualizado");
	exit(EXIT_FAILURE);
	return proceso; //no debería llegar acá
}





