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

    t_list* instrucciones = deserializar_instrucciones(a_recibir, size_payload);

	free(a_recibir);
	return instrucciones;
}

t_list* deserializar_instrucciones(void* a_recibir, int size_payload) {
    t_list* instrucciones = list_create();
    int desplazamiento;
    t_dictionary* diccionario_instrucciones = crear_diccionario_instrucciones();

	for(desplazamiento = 0; desplazamiento < size_payload;) {
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
		*desplazamiento += sizeof(size_t);

		char* nomb_param = malloc(largo_parametro);
		memcpy(nomb_param, a_recibir + *desplazamiento, largo_parametro);		//pongo nombre parametro
		parametro = strdup(nomb_param);
		*desplazamiento += largo_parametro;
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


void enviar_pcb(t_pcb* pcb, int socket_cpu) {
	size_t size_total;
	void* stream_pcb_a_enviar = serializar_pcb(pcb, &size_total);
	log_info(logger, "size_total: %d", (int)size_total);

	send(socket_cpu, stream_pcb_a_enviar, size_total, 0);

	free(stream_pcb_a_enviar);
}

void* serializar_pcb(t_pcb* pcb, size_t* size_total) {
	size_t size_payload = tamanio_payload_pcb(pcb);
	*size_total = size_payload + sizeof(op_code) + sizeof(size_payload);
	void* stream_pcb = malloc(*size_total);
	int desplazamiento = 0;

	//*** memcpy código de operación y tamanio de Payload ***//

    op_code op_code_a_enviar = PCB; //TODO: agregar PCB a op_code
    memcpy(stream_pcb, &(op_code_a_enviar), sizeof(op_code_a_enviar));								//pongo op_code
    desplazamiento += sizeof(op_code_a_enviar);

    memcpy(stream_pcb + desplazamiento, &(size_payload), sizeof(size_t));								//pongo op_code
    desplazamiento += sizeof(size_t);

	//*** memcpy Payload ***//

	memcpy(stream_pcb, &(pcb->pid), sizeof(pcb->pid));
	desplazamiento += sizeof(pcb->pid);

	memcpy_instrucciones(stream_pcb, pcb->instrucciones, &desplazamiento);

	memcpy(stream_pcb + desplazamiento, &(pcb->pc), sizeof(pcb->pc));
	desplazamiento += sizeof(pcb->pc);

	memcpy_registros(stream_pcb, pcb->registros_cpu, &desplazamiento); // TODO: fijarse si está bien

	memcpy_tabla_segmentos(stream_pcb, pcb->tabla_segmentos, &desplazamiento); //TODO: interpretar qué es t_segmento y hacer memcpy

	memcpy(stream_pcb + desplazamiento, &(pcb->estimado_prox_rafaga), sizeof(pcb->estimado_prox_rafaga));
	desplazamiento += sizeof(pcb->estimado_prox_rafaga);

	memcpy(stream_pcb + desplazamiento, &(pcb->estimado_llegada_ready), sizeof(pcb->estimado_llegada_ready));
	desplazamiento += sizeof(pcb->estimado_llegada_ready);

	memcpy_archivos_abiertos(stream_pcb, pcb->archivos_abiertos, &desplazamiento); //TODO: interpretar qué es t_archivo y hacer memcpy

	memcpy(stream_pcb + desplazamiento, &(pcb->socket_consola), sizeof(pcb->socket_consola));
	desplazamiento += sizeof(pcb->socket_consola);

	return stream_pcb;
}

size_t tamanio_payload_pcb(t_pcb* pcb) {
	size_t size = sizeof(pcb->pid) + tamanio_instrucciones(pcb->instrucciones) + sizeof(pcb->pc);
	size += sizeof(char)*16*7; // Por todos los char[] de los registros (?
	size += sizeof(pcb->estimado_prox_rafaga) + sizeof(pcb->estimado_llegada_ready) + sizeof(pcb->socket_consola);
	return size; // + sizeof(tabla_segmentos) + sizeof(archivos_abiertos);
	//TODO: los 2 sizeofs de arriba
}

size_t tamanio_instrucciones(t_list* instrucciones) {
	int size = 0;
	for(int i = 0; i < instrucciones->elements_count; i++) {
		t_instruccion* instruccion = (t_instruccion*)list_get(instrucciones, i);
	    size += sizeof(size_t)	//para longitud del nombre / para decir cuantos char son el nombre de instruccion
	    		+ strlen(instruccion->nombre) + 1
				+ tamanio_parametros(instruccion->parametros, i);
    }
	return size;
}

int tamanio_parametros(t_list* parametros, int index_instruccion) {
	int size_parametro = 0;
	for(int i = 0; i < parametros->elements_count; i++) {
	    size_parametro += sizeof(size_t)	//para decir cuantos char son el nombre de parametro
	    		+ strlen( (char*)list_get(parametros, i) ) + 1;
    }
	return size_parametro;
}

void memcpy_instrucciones(void* stream_pcb, t_list* instrucciones, int* desplazamiento) {
	for(int i = 0 ; i < instrucciones->elements_count ; i++) {
		t_instruccion* instruccion = (t_instruccion*)list_get(instrucciones, i);
		size_t largo_nombre = strlen(instruccion->nombre)+1;

		memcpy(stream_pcb + *desplazamiento, &(largo_nombre), sizeof(size_t));		//pongo size de nombre instruccion
		*desplazamiento += sizeof(size_t);

		memcpy(stream_pcb + *desplazamiento, instruccion->nombre, largo_nombre);		//pongo nombre instruccion
		*desplazamiento += largo_nombre;

		t_list* parametros = instruccion->parametros;
		for(int j = 0; j < parametros->elements_count; j++) {
			char* parametro = (char*)list_get(parametros, j);
			size_t largo_nombre = strlen(parametro)+1;

			memcpy(stream_pcb + *desplazamiento, &(largo_nombre), sizeof(size_t));	//pongo size nombre parametro
			*desplazamiento += sizeof(size_t);

			memcpy(stream_pcb + *desplazamiento, parametro, largo_nombre);			//pongo parametro
			*desplazamiento += largo_nombre;
		}
	}
}

void memcpy_registros(void* stream_pcb, t_registros_cpu registros_cpu, int* desplazamiento) {
	memcpy(stream_pcb + *desplazamiento, &(registros_cpu.AX), sizeof(registros_cpu.AX)); //Fijarse si se puede hacer pcb->registros_cpu.AX o hay que hacer un strdup antes o algo así.
	*desplazamiento += sizeof(registros_cpu.AX);

	memcpy(stream_pcb + *desplazamiento, &(registros_cpu.BX), sizeof(registros_cpu.BX));
	*desplazamiento += sizeof(registros_cpu.BX);

	memcpy(stream_pcb + *desplazamiento, &(registros_cpu.CX), sizeof(registros_cpu.CX));
	*desplazamiento += sizeof(registros_cpu.CX);

	memcpy(stream_pcb + *desplazamiento, &(registros_cpu.DX), sizeof(registros_cpu.DX));
	*desplazamiento += sizeof(registros_cpu.DX);

	memcpy(stream_pcb + *desplazamiento, &(registros_cpu.EAX), sizeof(registros_cpu.EAX));
	*desplazamiento += sizeof(registros_cpu.EAX);

	memcpy(stream_pcb + *desplazamiento, &(registros_cpu.EBX), sizeof(registros_cpu.EBX));
	*desplazamiento += sizeof(registros_cpu.EBX);

	memcpy(stream_pcb + *desplazamiento, &(registros_cpu.ECX), sizeof(registros_cpu.ECX));
	*desplazamiento += sizeof(registros_cpu.ECX);

	memcpy(stream_pcb + *desplazamiento, &(registros_cpu.EDX), sizeof(registros_cpu.EDX));
	*desplazamiento += sizeof(registros_cpu.EDX);

	memcpy(stream_pcb + *desplazamiento, &(registros_cpu.RAX), sizeof(registros_cpu.RAX));
	*desplazamiento += sizeof(registros_cpu.RAX);

	memcpy(stream_pcb + *desplazamiento, &(registros_cpu.RBX), sizeof(registros_cpu.RBX));
	*desplazamiento += sizeof(registros_cpu.RBX);

	memcpy(stream_pcb + *desplazamiento, &(registros_cpu.RCX), sizeof(registros_cpu.RCX));
	*desplazamiento += sizeof(registros_cpu.RCX);

	memcpy(stream_pcb + *desplazamiento, &(registros_cpu.RDX), sizeof(registros_cpu.RDX));
	*desplazamiento += sizeof(registros_cpu.RDX);
}

void memcpy_tabla_segmentos(void* stream, t_list* tabla_segmentos, int* desplazamiento) {
	for(int i = 0; i < tabla_segmentos->elements_count; i++) {
		//TODO: memcpy
		/*
		t_segmento* segmento = (t_segmento*)list_get(tabla_segmentos, i);
		memcpy(stream + *desplazamiento, segmento, sizeof(t_segmento));
		*desplazamiento += sizeof(t_segmento);
		*/
	}
}

void memcpy_archivos_abiertos(void* stream, t_list* archivos_abiertos, int* desplazamiento) {
	for(int i = 0; i < archivos_abiertos->elements_count; i++) {
		//TODO: memcpy
		/*
		t_archivo* archivo = (t_archivo*)list_get(archivos_abiertos, i);
		memcpy(stream + *desplazamiento, archivo, sizeof(t_archivo));
		*desplazamiento += sizeof(t_archivo);
		*/
	}
}

t_rta_cpu_al_kernel esperar_cpu(){
	t_rta_cpu_al_kernel respuesta;

	recv(socket_cpu, &respuesta, sizeof(t_rta_cpu_al_kernel), MSG_WAITALL); //va el MSG_WAITALL?

	return respuesta;
}
