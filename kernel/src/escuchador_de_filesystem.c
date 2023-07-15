#include "../include/escuchador_de_filesystem.h"

void escuchar_de_filesystem() {
	//Para no perder mensajes que llegan de FS en caso de que mande varios seguidos mientras Kernel está ejecutando
	while(1) {
		t_msj_kernel_fileSystem op_code_recibido = recibir_msj(socket_fileSystem);
		switch(op_code_recibido) {
			case EL_ARCHIVO_YA_EXISTE: case EL_ARCHIVO_NO_EXISTE: case EL_ARCHIVO_FUE_CREADO:

				respuesta_fs_global = op_code_recibido;
				sem_post(&sem_respuesta_fs);
				break;

			case EL_ARCHIVO_FUE_LEIDO: case EL_ARCHIVO_FUE_ESCRITO:
				pthread_mutex_lock(&mutex_cantidad_de_reads_writes);
				cantidad_de_reads_writes--;
				if(!cantidad_de_reads_writes) {
					sem_post(&sem_compactacion); //solo avisa que se puede hacer compactacion si no hay operaciones en proceso
				}
				pthread_mutex_unlock(&mutex_cantidad_de_reads_writes);

				//No debe haber break, porque de igual forma tiene que hacer exactamente lo mismo que el proximo case
			//no break
			case EL_ARCHIVO_FUE_TRUNCADO:
				char** parametros = recibir_parametros_de_mensaje(socket_fileSystem);

				//TODO: habría que usar otro mutex?? porque si se está leyendo ya se hace wait, por lo que no entraría acá hasta que se terminen de leer todos los archivos
				//Puede que se esté compactando y se estaría cambiando la lista de bloqueados.
				sem_wait(&sem_compactacion); //Funcionaría como un mutex en este caso
				desbloquear_pcb_por_archivo(parametros[0], atoi(parametros[1]));
				sem_post(&sem_compactacion); //Funcionaría como un mutex en este caso

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

	ready_list_push(pcb, "BLOCK");
}

t_pcb* obtener_pcb_de_cola(t_recurso* archivo, int pid) { //Busca al pcb en la cola y lo devuelve (sacandolo de la cola)
    t_pcb* pcb_aux;
    t_list* lista_aux = archivo->cola_bloqueados->elements;

    for(int i = 0; i < list_size(lista_aux); i++) {
        pcb_aux = list_get(lista_aux, i);
        if(pcb_aux->pid == pid) {
            list_remove(lista_aux, i);
            return pcb_aux;
        }
    }
    return NULL;
}
