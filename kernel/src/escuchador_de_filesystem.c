#include "../include/escuchador_de_filesystem.h"

void escuchar_de_filesystem() {
	while(1) {
		int op_code_recibido = recibir_msj(socket_fileSystem);
		switch(op_code_recibido){
			case EL_ARCHIVO_YA_EXISTE: case EL_ARCHIVO_NO_EXISTE: case EL_ARCHIVO_FUE_CREADO:

				rta_filesystem_global = op_code_recibido;
				sem_post(&sem_rta_filesystem);
				break;

			case EL_ARCHIVO_FUE_TRUNCADO: case EL_ARCHIVO_FUE_LEIDO: case EL_ARCHIVO_FUE_ESCRITO:

				char** parametros = recibir_parametros_de_mensaje(socket_fileSystem);
				desbloquear_pcb_por_archivo(parametros[0], atoi(parametros[1]));

				string_array_destroy(parametros);
				break;

			default:
				exit(EXIT_FAILURE);
		}
	}
}

void desbloquear_pcb_por_archivo(char* nombre_archivo, int pid) {
	t_recurso* archivo = buscar_recurso(nombre_archivo, list_archivos);
	archivo->cantidad_disponibles++;

	pthread_mutex_lock(&(archivo->mutex_archivo));
	t_pcb* pcb = obtener_pcb_de_cola(archivo, pid);
	pthread_mutex_unlock(&(archivo->mutex_archivo));

	ready_list_push(pcb);
}

t_pcb* obtener_pcb_de_cola(t_recurso* archivo, int pid) { //Busca al pcb en la cola y lo devuelve (sacandolo de la cola)
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
