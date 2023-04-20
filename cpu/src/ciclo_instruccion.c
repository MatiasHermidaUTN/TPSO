#include "../include/ciclo_instruccion.h"

void correr_ciclo_instruccion(int socket_kernel) {
	t_pcb* pcb = recibir_pcb(socket_kernel);

	//TODO: t_instruccion* instruccion_recibida = fetch_proxima_instruccion(pcb);

	//TODO: decode(instruccion_recibida);
}

t_pcb* recibir_pcb(int socket_kernel) {
	size_t size_payload;
    if (recv(socket_kernel, &size_payload, sizeof(size_t), 0) != sizeof(size_t)) {
        return false; //TODO: false? o exit(algo) ?
    }

	void* stream_pcb_a_recibir = malloc(size_payload);
    if (recv(socket_kernel, stream_pcb_a_recibir, size_payload, 0) != size_payload) {
        free(stream_pcb_a_recibir);
        return false;
    }

    t_pcb* pcb = deserializar_pcb(stream_pcb_a_recibir, size_payload);

	free(stream_pcb_a_recibir);
	return pcb;
}

t_pcb* deserializar_pcb(void* stream, size_t size_payload) {
	t_pcb* pcb = (t_pcb*)malloc(size_payload);
	int desplazamiento = 0;

	memcpy(&(pcb->pid), stream + desplazamiento, sizeof(pcb->pid));
	desplazamiento += sizeof(pcb->pid);

	pcb->instrucciones = deserializar_instrucciones(stream, size_payload, &desplazamiento);
	print_l_instrucciones(pcb->instrucciones);

	memcpy(&(pcb->pc), stream + desplazamiento, sizeof(pcb->pc));
	desplazamiento += sizeof(pcb->pc);

	memcpy_registros(&(pcb->registros_cpu), stream, &desplazamiento);

	memcpy_tabla_segmentos(pcb->tabla_segmentos, stream, &desplazamiento);

	memcpy(&(pcb->estimado_prox_rafaga), stream + desplazamiento, sizeof(pcb->estimado_prox_rafaga));
	desplazamiento += sizeof(pcb->estimado_prox_rafaga);

	memcpy(&(pcb->estimado_llegada_ready), stream + desplazamiento, sizeof(pcb->estimado_llegada_ready));
	desplazamiento += sizeof(pcb->estimado_llegada_ready);

	memcpy_archivos_abiertos(pcb->archivos_abiertos, stream, &desplazamiento);

	memcpy(&(pcb->socket_consola), stream + desplazamiento, sizeof(pcb->socket_consola));
	desplazamiento += sizeof(pcb->socket_consola);

	return pcb;
}

t_list* deserializar_instrucciones(void* a_recibir, size_t size_payload, int* desplazamiento) {
    t_list* instrucciones = list_create();
    t_dictionary* diccionario_instrucciones = crear_diccionario_instrucciones();

	while(*desplazamiento < size_payload) {
    	t_instruccion* instruccion = malloc(sizeof(t_instruccion));
		size_t largo_nombre;

		memcpy(&(largo_nombre), a_recibir + *desplazamiento, sizeof(size_t));		//pongo size de nombre instruccion
		*desplazamiento += sizeof(size_t);
	    //printf("%d\n", (int)largo_nombre);

	    char* nombre_instruccion = malloc(largo_nombre);
		memcpy(nombre_instruccion, a_recibir + *desplazamiento, largo_nombre);		//pongo nombre instruccion
	    instruccion->nombre = strdup(nombre_instruccion);
		*desplazamiento += largo_nombre;
	    free(nombre_instruccion);

	    //***PARAMETROS***
		instruccion->parametros = list_create();

		deserializar_parametros(a_recibir, desplazamiento, instruccion, diccionario_instrucciones);

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

		char* nomb_param = malloc(largo_parametro);
		memcpy(nomb_param, a_recibir + *desplazamiento, largo_parametro);		//pongo nombre parametro
		parametro = strdup(nomb_param);
		*desplazamiento+= largo_parametro;
		free(nomb_param);

		list_add(instruccion->parametros, parametro);
    }
}

void print_l_instrucciones(t_list* instrucciones) {
	for(int i = 0; i < instrucciones->elements_count; i++) {
		t_instruccion* instruccion = (t_instruccion*)list_get(instrucciones, i);
	    printf("%s", instruccion->nombre);

	    for(int i = 0; i < instruccion->parametros->elements_count; i++) {
			char* parametro = (char*)list_get(instruccion->parametros, i);
		    printf(" %s", parametro);
	    }
	    printf("\n");
    }
}

void memcpy_registros(t_registros_cpu* registros_cpu, void* stream_pcb, int* desplazamiento) {
	memcpy(registros_cpu->AX, stream_pcb + *desplazamiento, sizeof(registros_cpu->AX)); //Fijarse si se puede hacer pcb->registros_cpu.AX o hay que hacer un strdup antes o algo así.
	*desplazamiento += sizeof(registros_cpu->AX);

	memcpy(registros_cpu->BX, stream_pcb + *desplazamiento, sizeof(registros_cpu->BX));
	*desplazamiento += sizeof(registros_cpu->BX);

	memcpy(registros_cpu->CX, stream_pcb + *desplazamiento, sizeof(registros_cpu->CX));
	*desplazamiento += sizeof(registros_cpu->CX);

	memcpy(registros_cpu->DX, stream_pcb + *desplazamiento, sizeof(registros_cpu->DX));
	*desplazamiento += sizeof(registros_cpu->DX);

	memcpy(registros_cpu->EAX, stream_pcb + *desplazamiento, sizeof(registros_cpu->EAX));
	*desplazamiento += sizeof(registros_cpu->EAX);

	memcpy(registros_cpu->EBX, stream_pcb + *desplazamiento, sizeof(registros_cpu->EBX));
	*desplazamiento += sizeof(registros_cpu->EBX);

	memcpy(registros_cpu->ECX, stream_pcb + *desplazamiento, sizeof(registros_cpu->ECX));
	*desplazamiento += sizeof(registros_cpu->ECX);

	memcpy(registros_cpu->EDX, stream_pcb + *desplazamiento, sizeof(registros_cpu->EDX));
	*desplazamiento += sizeof(registros_cpu->EDX);

	memcpy(registros_cpu->RAX, stream_pcb + *desplazamiento, sizeof(registros_cpu->RAX));
	*desplazamiento += sizeof(registros_cpu->RAX);

	memcpy(registros_cpu->RBX, stream_pcb + *desplazamiento, sizeof(registros_cpu->RBX));
	*desplazamiento += sizeof(registros_cpu->RBX);

	memcpy(registros_cpu->RCX, stream_pcb + *desplazamiento, sizeof(registros_cpu->RCX));
	*desplazamiento += sizeof(registros_cpu->RCX);

	memcpy(registros_cpu->RDX, stream_pcb + *desplazamiento, sizeof(registros_cpu->RDX));
	*desplazamiento += sizeof(registros_cpu->RDX);
}

void memcpy_tabla_segmentos(t_list* tabla_segmentos, void* stream, int* desplazamiento) {
	for(int i = 0; i < tabla_segmentos->elements_count; i++) {
		//TODO: memcpy
		/*
		t_segmento* segmento = (t_segmento*)list_get(tabla_segmentos, i);
		memcpy(segmento, stream + *desplazamiento, sizeof(t_segmento));
		*desplazamiento += sizeof(t_segmento);
		*/
	}
}

void memcpy_archivos_abiertos(t_list* archivos_abiertos, void* stream, int* desplazamiento) {
	for(int i = 0; i < archivos_abiertos->elements_count; i++) {
		//TODO: memcpy
		/*
		t_archivo* archivo = (t_archivo*)list_get(archivos_abiertos, i);
		memcpy(archivo, stream + *desplazamiento, sizeof(t_archivo));
		*desplazamiento += sizeof(t_archivo);
		*/
	}
}

