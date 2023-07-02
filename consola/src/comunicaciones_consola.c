#include "../include/comunicaciones_consola.h"

int enviar_instrucciones(t_list* instrucciones) {
	int size = tamanio_msj(instrucciones);
	void* a_enviar = serializar_instrucciones(instrucciones, size);

	send(socket_kernel, a_enviar, size, 0);

	free(a_enviar);
	return 0;
}

void* serializar_instrucciones(t_list* instrucciones, int size) {
    void* stream = malloc(size);
    size_t size_payload = size - sizeof(t_msj_kernel_consola) - sizeof(size_t);
    int desplazamiento = 0;

    t_msj_kernel_consola op_code = LIST_INSTRUCCIONES;
    memcpy(stream + desplazamiento, &(op_code), sizeof(op_code));
    desplazamiento += sizeof(op_code);

    memcpy(stream + desplazamiento, &size_payload, sizeof(size_t));
    desplazamiento += sizeof(size_t);

    for(int i = 0; i < list_size(instrucciones); i++) {
		t_instruccion* instruccion = list_get(instrucciones, i);

		size_t largo_nombre = strlen(instruccion->nombre) + 1;
		memcpy(stream + desplazamiento, &largo_nombre, sizeof(size_t));
		desplazamiento += sizeof(size_t);

		memcpy(stream + desplazamiento, instruccion->nombre, largo_nombre);
		desplazamiento += largo_nombre;

		t_list* parametros = instruccion->parametros;
		for(int j = 0; j < list_size(parametros); j++) {
			char* parametro = list_get(parametros, j);

			size_t largo_nombre = strlen(parametro) + 1;
			memcpy(stream + desplazamiento, &largo_nombre, sizeof(size_t));
			desplazamiento += sizeof(size_t);

			memcpy(stream + desplazamiento, parametro, largo_nombre);
			desplazamiento += largo_nombre;
	    }
	}

    return stream;
}

int tamanio_msj(t_list* instrucciones) {
	int size = 0;
    size += sizeof(int);
    size += sizeof(size_t);

	for(int i = 0; i < list_size(instrucciones); i++) {
		t_instruccion* instruccion = list_get(instrucciones, i);

	    size += sizeof(size_t)	//para decir cuantos char son el nombre de instruccion
	    		+ strlen(instruccion->nombre) + 1
				+ calculo_tamanio_parametros(instruccion->parametros, i);
    }

	return size;
}

int calculo_tamanio_parametros(t_list* parametros, int index_instruccion) {
	int size_parametros = 0;
	for(int i = 0; i < list_size(parametros); i++) {
	    size_parametros += sizeof(size_t)	//para decir cuantos char son el nombre de parametro
	    		+ strlen(((char*)list_get(parametros, i))) + 1;
    }

	return size_parametros;
}
