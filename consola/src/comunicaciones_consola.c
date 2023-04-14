#include "../include/comunicaciones_consola.h"

int enviar_instrucciones(int socket_kernel, t_list* instrucciones){
	int size = calculo_tamanio_msj(instrucciones);
    //printf("Los bytes con instrucciones son: %d\n", size);
	void* a_enviar = serializar_instrucciones(instrucciones, size);

	send(socket_kernel, a_enviar, size, 0);

	free(a_enviar);
	return 0;
}

void* serializar_instrucciones(t_list* instrucciones, int size){
    void* stream = malloc(size);
    size_t size_payload = size - sizeof(op_code) - sizeof(size_t);
    int desplazamiento = 0;

    //copio op code de msj
    op_code op_code1 = LIST_INSTRUCCIONES;
    memcpy(stream, &(op_code1), sizeof(op_code));								//pongo op_code
    desplazamiento += sizeof(op_code);
    memcpy(stream + desplazamiento, &(size_payload), sizeof(size_t));								//pongo op_code
    desplazamiento += sizeof(size_t);

    for(int i = 0 ; i < instrucciones->elements_count ; i++){
		t_instruccion* instruccion = (t_instruccion*)list_get(instrucciones, i);
		size_t largo_nombre = strlen(instruccion->nombre)+1;
		memcpy(stream + desplazamiento, &(largo_nombre), sizeof(size_t));		//pongo size de nombre instruccion
		desplazamiento+= sizeof(size_t);
		memcpy(stream + desplazamiento, instruccion->nombre, largo_nombre);		//pongo nombre instruccion
		desplazamiento+= largo_nombre;

		t_list* parametros = instruccion->parametros;
		for(int j = 0; j < parametros->elements_count; j++){
			char* parametro = (char*)list_get(parametros, j);
			size_t largo_nombre = strlen(parametro)+1;
			memcpy(stream + desplazamiento, &(largo_nombre), sizeof(size_t));	//pongo size nombre parametro
			desplazamiento+= sizeof(size_t);
			memcpy(stream + desplazamiento, parametro, largo_nombre);			//pongo parametro
			desplazamiento+= largo_nombre;
	    }
	}
    printf("Los bytes en el stream son: %d\n", desplazamiento);

    return stream;
}

int calculo_tamanio_msj(t_list* instrucciones){
	int size = 0;
    size += sizeof(op_code);
    size += sizeof(size_t);
	for(int i = 0; i < instrucciones->elements_count; i++){
		t_instruccion* instruccion = (t_instruccion*)list_get(instrucciones, i);
	    size += sizeof(size_t)	//para decir cuantos char son el nombre de instruccion
	    		+ strlen(instruccion->nombre) + 1
				+ calculo_tamanio_parametros(instruccion->parametros, i);
    }
	return size;
}

int calculo_tamanio_parametros(t_list* parametros, int index_instruccion){
	int size_param = 0;
	for(int i = 0; i < parametros->elements_count; i++){
	    size_param += sizeof(size_t)	//para decir cuantos char son el nombre de parametro
	    		+ strlen(((char*)list_get(parametros, i))) + 1;
    }
	return size_param;
}
