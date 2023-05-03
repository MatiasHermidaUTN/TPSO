#include "../include/comunicaciones_kernel.h"

t_list* recibir_instrucciones(int socket_consola) {
	size_t size_payload;
    if (recv(socket_consola, &size_payload, sizeof(size_t), 0) != sizeof(size_t)) {
        return false;
    }

	void* a_recibir = malloc(size_payload);

    if (recv(socket_consola, a_recibir, size_payload, 0) != size_payload) {
        free(a_recibir);
        return false;
    }

    t_list* instrucciones = deserializar_instrucciones_kernel(a_recibir, size_payload);

	free(a_recibir);
	return instrucciones;
}

t_list* deserializar_instrucciones_kernel(void* a_recibir, int size_payload) {
    t_list* instrucciones = list_create();
    int desplazamiento;
    t_dictionary* diccionario_instrucciones = crear_diccionario_instrucciones();

	for(desplazamiento = 0 ; desplazamiento < size_payload ;) {
    	t_instruccion* instruccion = malloc(sizeof(t_instruccion));
		size_t largo_nombre;

		memcpy(&(largo_nombre), a_recibir + desplazamiento, sizeof(size_t));		//pongo size de nombre instruccion
		desplazamiento += sizeof(size_t);
	    //printf("%d\n", (int)largo_nombre);

	    char* nombre_instruccion = malloc(largo_nombre);
		memcpy(nombre_instruccion, a_recibir + desplazamiento, largo_nombre);		//pongo nombre instruccion
	    instruccion->nombre = strdup(nombre_instruccion);
		desplazamiento += largo_nombre;
	    free(nombre_instruccion);

	    //***PARAMETROS***
		instruccion->parametros = list_create();

		deserializar_parametros(a_recibir, &desplazamiento, instruccion, diccionario_instrucciones);

		list_add(instrucciones, instruccion);
	}

    //printf("Los bytes recibidos en el stream son: %d\n", desplazamiento+(int)sizeof(op_code)+(int)sizeof(size_t));

    dictionary_destroy_and_destroy_elements(diccionario_instrucciones, (void*)destruir_diccionario);

    return instrucciones;
}

void deserializar_parametros(void* a_recibir, int* desplazamiento, t_instruccion* instruccion, t_dictionary* diccionario_instrucciones) {
	int* cantidadDeParametrosEsperados;
	if(!dictionary_has_key(diccionario_instrucciones, instruccion->nombre)) {
		//log_error(logger, "No existe instruccion: %s , en el diccionario", instruccion->nombre);
		printf("No existe instruccion: %s , en el diccionario", instruccion->nombre);
		exit(-1);
	}
	cantidadDeParametrosEsperados = (int*)dictionary_get(diccionario_instrucciones, instruccion->nombre);

	for(int j = 0; j < *cantidadDeParametrosEsperados; j++) {
		char* parametro;
		size_t largo_parametro;

		memcpy(&(largo_parametro), a_recibir + *desplazamiento, sizeof(size_t));		//pongo size de nombre parametro
		*desplazamiento+= sizeof(size_t);

		char* nombre_paramtro = malloc(largo_parametro);
		memcpy(nombre_paramtro, a_recibir + *desplazamiento, largo_parametro);		//pongo nombre parametro
		parametro = strdup(nombre_paramtro);
		*desplazamiento += largo_parametro;
		free(nombre_paramtro);

		list_add(instruccion->parametros, parametro);
    }
}

t_msj_kernel_cpu esperar_cpu() {
	t_msj_kernel_cpu respuesta;
	recv(socket_cpu, &respuesta, sizeof(t_msj_kernel_cpu), MSG_WAITALL); //va el MSG_WAITALL?
	return respuesta;
}

char** recibir_parametros_de_instruccion() {
	size_t cantidad_de_parametros;
	size_t tamanio_parametro;

	recv(socket_cpu, &cantidad_de_parametros, sizeof(size_t), MSG_WAITALL);
	char** parametros = malloc(cantidad_de_parametros * sizeof(char*));

	for(int i = 0; i < cantidad_de_parametros; i++) {
		recv(socket_cpu, &tamanio_parametro, sizeof(size_t), MSG_WAITALL);

		parametros[i] = malloc(tamanio_parametro);
		recv(socket_cpu, parametros[i], tamanio_parametro, MSG_WAITALL);
	}

	/*
	char** parametros = string_array_new(); //Hace malloc(sizeof(char*))

	recv(socket_cpu, &cantidad_de_parametros, sizeof(size_t), MSG_WAITALL);

	for(int i = 0; i < cantidad_de_parametros; i++) {
		recv(socket_cpu, &tamanio_parametro, sizeof(size_t), MSG_WAITALL);

		char* parametro = malloc(tamanio_parametro);
		recv(socket_cpu, parametro, tamanio_parametro, MSG_WAITALL);
		string_array_push(&parametros, parametro); //Hace malloc del nuevo parametro y realloc del char**
		free(parametro);
	}
	*/

	return parametros; //acordarse de hacerle el free del otro lado
}
