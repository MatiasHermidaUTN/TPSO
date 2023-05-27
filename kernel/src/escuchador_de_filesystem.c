#include "../include/escuchador_de_filesystem.h"

void escuchar_de_filesystem(){
	while(1){
		int op_code_recibido = recibir_msj(socket_fileSystem);
		switch(op_code_recibido){
			case EL_ARCHIVO_YA_EXISTE:
				rta_filesystem_global = EL_ARCHIVO_YA_EXISTE;
				sem_post(&sem_rta_filesystem);
				break;
			case EL_ARCHIVO_NO_EXISTE:
				rta_filesystem_global = EL_ARCHIVO_NO_EXISTE;
				sem_post(&sem_rta_filesystem);
				break;
			case EL_ARCHIVO_FUE_CREADO:
				rta_filesystem_global = EL_ARCHIVO_FUE_CREADO;
				sem_post(&sem_rta_filesystem);
				break;
			case EL_ARCHIVO_FUE_TRUNCADO:
				char** parametros_truncar;
				parametros_truncar = recibir_parametros_de_mensaje(socket_fileSystem);
				desbloquear_pcb_por_archivo(parametros_truncar[0], atoi(parametros_truncar[1]));
				string_array_destroy(parametros_truncar);

				break;

			case EL_ARCHIVO_FUE_LEIDO:
				char** parametros_leer;
				parametros_leer = recibir_parametros_de_mensaje(socket_fileSystem);
				desbloquear_pcb_por_archivo(parametros_leer[0], atoi(parametros_leer[1]));
				string_array_destroy(parametros_leer);
				break;

			case EL_ARCHIVO_FUE_ESCRITO:
				char** parametros_escribir;
				parametros_escribir = recibir_parametros_de_mensaje(socket_fileSystem);
				desbloquear_pcb_por_archivo(parametros_escribir[0], atoi(parametros_escribir[1]));
				string_array_destroy(parametros_escribir);
				break;

			default:
				exit(-1);
		}
	}
}

t_pcb* obtener_pcb_de_cola(t_recurso* archivo, int pid){ //Busca al pcb en la cola y lo devuelve (sacandolo de la cola)
	t_pcb* pcb_a_devolver = NULL;
	t_pcb* pcb_aux;

	for(int i = 0; i < queue_size(archivo->cola_bloqueados); i++) {
		pcb_aux = queue_pop(archivo->cola_bloqueados);

		if(pcb_aux->pid == pid) {
			pcb_a_devolver = pcb_aux;
		}
		else { //Tiene que seguir popeando y pusheando el resto de pcbs para que la cola quede en el mismo orden
			queue_push(archivo->cola_bloqueados, pcb_aux);
		}
	}
	return pcb_a_devolver;
}

void desbloquear_pcb_por_archivo(char* nombre_archivo,int pid){
	t_recurso* archivo = buscar_recurso(nombre_archivo, list_archivos);
	archivo->cantidad_disponibles++;
	int pos = obtener_posicion_recurso(list_archivos, archivo);
	pthread_mutex_t* mutex = list_get(mutex_list_archivos, pos);
	pthread_mutex_lock(mutex);
	t_pcb* pcb = obtener_pcb_de_cola(archivo, pid);
	pthread_mutex_unlock(mutex);
	log_info(logger, "PID: %d - Estado Anterior: BLOCK - Estado Actual: READY", pid); //log obligatorio
	ready_list_push(pcb);

}
