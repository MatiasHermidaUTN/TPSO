#include "manejar_mensajes.h"

void escuchar_kernel() {
	while(1){
		t_mensajes* args = malloc(sizeof(t_mensajes));
		args->cod_op = recibir_msj(kernel);
		args->parametros = recibir_parametros_de_mensaje(kernel);
		list_push_con_mutex(lista_fifo_msj, args, &mutex_cola_msj);
		sem_post(&sem_sincro_cant_msj);
	}
	return;
}

int manejar_mensaje(){
	sem_wait(&sem_sincro_cant_msj);

	t_mensajes* args = list_pop_con_mutex(lista_fifo_msj, &mutex_cola_msj);

	char* nombre_archivo;
	int nuevo_tamanio_archivo;
	int apartir_de_donde_X;
	int cuanto_X;
	int dir_fisica_memoria;

	char* buffer;
	switch (args->cod_op) {
		case EXISTE_ARCHIVO:
			nombre_archivo = args->parametros[0];
			printf("abrir nombre_archivo: %s\n",nombre_archivo);
			if (existe_archivo(nombre_archivo)) {	//existe FCB?
				enviar_msj(kernel, EL_ARCHIVO_YA_EXISTE);
			} else {
				enviar_msj(kernel, EL_ARCHIVO_NO_EXISTE);
			}
			printf("\n");
			break;

		case CREAR_ARCHIVO:
			nombre_archivo = args->parametros[0];
			printf("crear nombre_archivo: %s \n", nombre_archivo);
			crear_archivo(nombre_archivo);	//crear FCB y poner tamaño 0 y sin bloques asociados.
			enviar_msj(kernel, EL_ARCHIVO_FUE_CREADO);
			printf("archivo creado: %s\n",nombre_archivo);
			printf("unos dsp crear: %d\n", cant_unos_en_bitmap());
			printf("\n");
			break;

		case TRUNCAR_ARCHIVO:
			nombre_archivo = args->parametros[0];
			nuevo_tamanio_archivo = atoi(args->parametros[1]);
			char* pid_truncar = args->parametros[2];

			printf("truncar nombre_archivo: %s\n", nombre_archivo);
			printf("nuevo_tamanio_archivo: %d\n", nuevo_tamanio_archivo);
			printf("unos antes truncar: %d\n", cant_unos_en_bitmap());
			truncar(nombre_archivo, nuevo_tamanio_archivo);

			char ** parametros_a_enviar = string_array_new();
			string_array_push(&parametros_a_enviar, nombre_archivo);
			string_array_push(&parametros_a_enviar, pid_truncar);
			enviar_msj_con_parametros(kernel, EL_ARCHIVO_FUE_TRUNCADO, parametros_a_enviar);
			free(parametros_a_enviar);
			printf("\n");
			break;

		case LEER_ARCHIVO:
			nombre_archivo = args->parametros[0];
			dir_fisica_memoria = atoi(args->parametros[1]);
			cuanto_X = atoi(args->parametros[2]);
			apartir_de_donde_X = atoi(args->parametros[3]);
			char* pid_leer = args->parametros[4];

			printf("leer nombre_archivo: %s\n", nombre_archivo);
			printf("apartir_de_donde_leer: %d\n", apartir_de_donde_X);
			printf("cuanto_leer: %d\n", cuanto_X);
			printf("dir_fisica_memoria: %d\n", dir_fisica_memoria);
			buffer = leer_archivo(nombre_archivo, apartir_de_donde_X, cuanto_X);	//malloc se hace en leer_archivo
			mandar_a_memoria(dir_fisica_memoria, cuanto_X, buffer);

			char** parametros_a_enviar_leer = string_array_new();
			string_array_push(&parametros_a_enviar_leer, nombre_archivo); //nombre del archivo
			string_array_push(&parametros_a_enviar_leer, pid_leer); //pid para desbloquearlo despues
			enviar_msj_con_parametros(kernel, EL_ARCHIVO_FUE_LEIDO, parametros_a_enviar_leer);
			free(parametros_a_enviar_leer);

			printf("\n");
			printf("leido: ");
			for(int i = 0 ; i < cuanto_X ; i++){
				printf("%c", buffer[i]);
			}
			printf("\n");
			printf("\n");
			free(buffer);
			break;

		case ESCRIBIR_ARCHIVO:
			nombre_archivo = args->parametros[0];
			dir_fisica_memoria = atoi(args->parametros[1]);
			cuanto_X = atoi(args->parametros[2]);
			apartir_de_donde_X = atoi(args->parametros[3]);
			char* pid_escribir = args->parametros[4];

			printf("escribir nombre_archivo: %s\n", nombre_archivo);
			printf("apartir_de_donde_escribir: %d\n", apartir_de_donde_X);
			printf("cuanto_escribir: %d\n", cuanto_X);
			printf("dir_fisica_memoria: %d\n", dir_fisica_memoria);
			buffer = escribir_en_memoria(dir_fisica_memoria, cuanto_X);	//malloc se hace en leer_de_memoria
			escribir_archivo(buffer, nombre_archivo, apartir_de_donde_X, cuanto_X);

			char** parametros_a_enviar_escribir = string_array_new();
			string_array_push(&parametros_a_enviar_escribir, args->parametros[0]); //nombre del archivo
			string_array_push(&parametros_a_enviar_escribir, pid_escribir); //pid para desbloquearlo despues
			enviar_msj_con_parametros(kernel, EL_ARCHIVO_FUE_ESCRITO, parametros_a_enviar_escribir);
			free(parametros_a_enviar_escribir);

			printf("\n");
			printf("escrito: %s\n", buffer);
			printf("\n");
			free(buffer);
			break;
		default:
			return 0;	//?
	}

	string_array_destroy(args->parametros);
	free(args);

	return 1;
}