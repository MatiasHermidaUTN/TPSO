#include "../include/comunicaciones_kernel.h"

t_list* recibir_instrucciones(int socket_consola){
	size_t size_payload;
    if (recv(socket_consola, &size_payload, sizeof(size_t), 0) != sizeof(size_t)){
        return false;
    }

	void* a_recibir = malloc(size_payload);

    if (recv(socket_consola, a_recibir, size_payload, 0) != size_payload) {
        free(a_recibir);
        return false;
    }

    t_list* instrucciones = deserializar_instrucciones(a_recibir, size_payload);

	free(a_recibir);
	return instrucciones;
}

t_list* deserializar_instrucciones(void* a_recibir, int size_payload){
    t_list* instrucciones = list_create();
    int desplazamiento;
    t_dictionary* diccionario_instrucciones = crear_diccionario_instrucciones();

	for(desplazamiento = 0 ; desplazamiento < size_payload ;){
    	t_instruccion* instruccion = malloc(sizeof(t_instruccion));
		size_t largo_nombre;

		memcpy(&(largo_nombre), a_recibir + desplazamiento, sizeof(size_t));		//pongo size de nombre instruccion
		desplazamiento+= sizeof(size_t);
	    //printf("%d\n", (int)largo_nombre);

	    char* nomb_instrucc = malloc(largo_nombre);
		memcpy(nomb_instrucc, a_recibir + desplazamiento, largo_nombre);		//pongo nombre instruccion
	    instruccion->nombre = strdup(nomb_instrucc);
		desplazamiento+= largo_nombre;
	    free(nomb_instrucc);

	    //***PARAMETROS***
		instruccion->parametros = list_create();

		deserializar_parametros(a_recibir, &desplazamiento, instruccion, diccionario_instrucciones);

		list_add(instrucciones, instruccion);
	}

    //printf("Los bytes recibidos en el stream son: %d\n", desplazamiento+(int)sizeof(op_code)+(int)sizeof(size_t));

    dictionary_destroy_and_destroy_elements(diccionario_instrucciones, (void*)destruir_diccionario);

    return instrucciones;
}

void deserializar_parametros(void* a_recibir, int* desplazamiento, t_instruccion* instruccion, t_dictionary* diccionario_instrucciones){
	int* cantidadDeParametrosEsperados;
	if(!dictionary_has_key(diccionario_instrucciones, instruccion->nombre)){
		//log_error(logger, "No existe instruccion: %s , en el diccionario", instruccion->nombre);
		printf("No existe instruccion: %s , en el diccionario", instruccion->nombre);
		exit(-1);
	}
	cantidadDeParametrosEsperados = (int*)dictionary_get(diccionario_instrucciones, instruccion->nombre);

	for(int j = 0; j < *cantidadDeParametrosEsperados; j++){
		char* parametro;
		size_t largo_parametro;

		memcpy(&(largo_parametro), a_recibir + *desplazamiento, sizeof(size_t));		//pongo size de nombre parametro
		*desplazamiento+= sizeof(size_t);

		char* nomb_param = malloc(largo_parametro);
		memcpy(nomb_param, a_recibir + *desplazamiento, largo_parametro);		//pongo nombre parametro
		parametro = strdup(nomb_param);
		*desplazamiento+= largo_parametro;
		free(nomb_param);

		list_add(instruccion->parametros, parametro);
    }

	return;
}

void print_l_instrucciones(t_list* instrucciones){
	for(int i = 0; i < instrucciones->elements_count; i++){
		t_instruccion* instruccion = (t_instruccion*)list_get(instrucciones, i);
	    printf("%s", instruccion->nombre);
		for(int i = 0; i < instruccion->parametros->elements_count; i++){
			char* parametro = (char*)list_get(instruccion->parametros, i);
		    printf(" %s", parametro);
	    }
	    printf("\n");
    }
}

void enviar_pcb(t_pcb* pcb){
	//TODO: IMPLEMENTAR
	return;
}

t_msj_kernel_cpu esperar_cpu(){
	//TODO: IMPLEMENTAR
	return YIELD_EJECUTADO;
}
