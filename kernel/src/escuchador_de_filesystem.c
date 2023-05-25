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
				rta_filesystem_global = EL_ARCHIVO_YA_EXISTE;
				sem_post(&sem_rta_filesystem);
				break;
			case EL_ARCHIVO_FUE_CREADO:
				rta_filesystem_global = EL_ARCHIVO_YA_EXISTE;
				sem_post(&sem_rta_filesystem);
				break;
			case EL_ARCHIVO_FUE_TRUNCADO:
				char** parametros;
				parametros = recibir_parametros_de_mensaje(socket_fileSystem);
				t_recurso* archivo = buscar_recurso(parametros[0], list_archivos);
				t_pcb* pcb = obtener_pcb_de_cola(archivo, atoi(parametros[1]));

				ready_list_push(pcb);
				string_array_destroy(parametros);

				break;
			default:
				exit(-1);
		}
	}
}

t_pcb* obtener_pcb_de_cola(t_recurso* archivo, int pid){ //Busca al pcb en la cola y lo devuelve (sacandolo de la cola)
	t_pcb* pcb_a_devolver = NULL;
	t_pcb* pcb_aux;
	int tamanio_cola = queue_size(archivo->cola_bloqueados);
	for(int i = 0; i< tamanio_cola; i++){
		pcb_aux = queue_pop(archivo->cola_bloqueados);

		log_warning(logger, "PID PCB AUX= %d", pcb_aux->pid);
		log_warning(logger, "PID PARAMETRO= %d", pid);

		if(pcb_aux->pid == pid){
			pcb_a_devolver = pcb_aux;
		}
		else{
			queue_push(archivo->cola_bloqueados, pcb_aux);
		}
	}
	return pcb_a_devolver;
}
