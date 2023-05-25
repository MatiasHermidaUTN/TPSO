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
				//char** parametros;
				//parametros = recibir_parametros_de_mensaje(socket_fileSystem);
				//t_recurso* recurso = buscar_recurso(parametros[0], list_archivos);

				//string_array_destroy(parametros);
				break;

			default:
				exit(-1);
		}
	}
}
