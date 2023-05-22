#include "utils.h"

//////////////////////////
// Funciones de cliente //
//////////////////////////

int crear_conexion(char *ip, char* puerto) {
	struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(ip, puerto, &hints, &server_info);

	int socket_cliente = socket(server_info->ai_family,
	                    	    server_info->ai_socktype,
								server_info->ai_protocol);

	if(connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen)) {
		freeaddrinfo(server_info);
		return -1;
	}

	freeaddrinfo(server_info);

	return socket_cliente;
}

void enviar_mensaje(char* mensaje, int socket_cliente)
{
	t_paquete* paquete = malloc(sizeof(t_paquete));

	paquete->codigo_operacion = MENSAJE;
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = strlen(mensaje) + 1;
	paquete->buffer->stream = malloc(paquete->buffer->size);
	memcpy(paquete->buffer->stream, mensaje, paquete->buffer->size);

	int bytes = paquete->buffer->size + 2*sizeof(int);

	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
	eliminar_paquete(paquete);
}

///////////////////////////
// Funciones de servidor //
///////////////////////////

int iniciar_servidor(char* IP, char* PUERTO) {
	int socket_servidor;

	struct addrinfo hints, *servinfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;


	getaddrinfo(IP, PUERTO, &hints, &servinfo);

	// Creamos el socket de escucha del servidor

	socket_servidor = socket(servinfo->ai_family,
							 servinfo->ai_socktype,
							 servinfo->ai_protocol);

	int my_true = 1; //Defino un true para poder pasarle el puntero al true
	setsockopt(socket_servidor, SOL_SOCKET, SO_REUSEADDR, &my_true, sizeof(int)); //Para cerrar el socket en cuanto se termine el proceso

	// Asociamos el socket a un puerto

	bind(socket_servidor, servinfo->ai_addr, servinfo->ai_addrlen);

	// Escuchamos las conexiones entrantes

	listen(socket_servidor, SOMAXCONN);

	freeaddrinfo(servinfo);
	//log_trace(logger, "Listo para escuchar a mi cliente");

	return socket_servidor;
}

int esperar_cliente(int socket_servidor) {
	int socket_cliente = accept(socket_servidor, NULL, NULL);
	return socket_cliente;
}

int recibir_operacion(int socket_cliente) {
	int cod_op;
	if(recv(socket_cliente, &cod_op, sizeof(op_code), MSG_WAITALL) > 0)
		return cod_op;
	else {
		close(socket_cliente);
		return -1;
	}
}


void* serializar_paquete(t_paquete* paquete, int bytes) {
    void * magic = malloc(bytes);
    int desplazamiento = 0;

    memcpy(magic + desplazamiento, &(paquete->codigo_operacion), sizeof(int));
    desplazamiento+= sizeof(int);
    memcpy(magic + desplazamiento, &(paquete->buffer->size), sizeof(int));
    desplazamiento+= sizeof(int);
    memcpy(magic + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
    desplazamiento+= paquete->buffer->size;

    return magic;
}

void recibir_mensaje(int socket_cliente) {
	int size;
	char* buffer = recibir_buffer(&size, socket_cliente);
	//log_info(logger, "Me llego el mensaje %s", buffer);
	puts(buffer);

	free(buffer);
}

void* recibir_buffer(int* size, int socket_cliente) {
    void * buffer;

    recv(socket_cliente, size, sizeof(int), MSG_WAITALL);
    buffer = malloc(*size);
    recv(socket_cliente, buffer, *size, MSG_WAITALL);

    return buffer;
}

void eliminar_paquete(t_paquete* paquete) {
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);
}

t_config* iniciar_config(char* path) {
	t_config* nuevo_config;

	nuevo_config = config_create(path);

	if(!nuevo_config) {
		printf("No se pudo crear el config.\n");
		exit(EXIT_FAILURE);
	}

	return nuevo_config;
}

t_log* iniciar_logger(char* path, char* nombre) {
	t_log* nuevo_logger = log_create(path, nombre, 1, LOG_LEVEL_INFO);

	if(!nuevo_logger) {
		printf("No se pudo crear un logger.");
		exit(EXIT_FAILURE);
	}

	return nuevo_logger;
}

t_handshake recibir_handshake(int socket_cliente) {
	t_handshake respuesta_handshake;
	recv(socket_cliente, &respuesta_handshake, sizeof(respuesta_handshake), MSG_WAITALL);
	return respuesta_handshake;
}

void enviar_handshake(int socket, t_handshake mensaje_handshake) {
	//TODO: Falta fijarse si da error
	send(socket, &mensaje_handshake, sizeof(mensaje_handshake), 0);
}

t_msj_kernel_consola recibir_fin_proceso(int socket_cliente) {
	t_msj_kernel_consola mensaje;
	recv(socket_cliente, &mensaje, sizeof(t_msj_kernel_consola), MSG_WAITALL);
	return mensaje;
}

void enviar_fin_proceso(int socket, t_msj_kernel_consola mensaje) {
	//TODO: Falta fijarse si da error
	send(socket, &mensaje, sizeof(mensaje), 0);
}

void *queue_pop_con_mutex(t_queue* queue, pthread_mutex_t* mutex) {
    pthread_mutex_lock(mutex);
    void *elemento = queue_pop(queue);
    pthread_mutex_unlock(mutex);
    return elemento;
}

void queue_push_con_mutex(t_queue* queue,void* elemento , pthread_mutex_t* mutex) {
    pthread_mutex_lock(mutex);
    queue_push(queue, elemento);
    pthread_mutex_unlock(mutex);
    return;
}

void *list_pop_con_mutex(t_list* lista, pthread_mutex_t* mutex) {
    pthread_mutex_lock(mutex);
    void* elemento = list_remove(lista, 0);
    pthread_mutex_unlock(mutex);
    return elemento;
}

void list_push_con_mutex(t_list* lista,void* elemento , pthread_mutex_t* mutex) {
    pthread_mutex_lock(mutex);
    list_add(lista, elemento);
    pthread_mutex_unlock(mutex);
    return;
}

void destruir_instruccion(t_instruccion* instruccion) {
	free(instruccion->nombre);
	list_destroy_and_destroy_elements(instruccion->parametros, (void*)free);
	free(instruccion);
}

void liberar_pcb(t_pcb* pcb) {
	list_destroy_and_destroy_elements(pcb->instrucciones, (void*)destruir_instruccion);
	//free(pcb->tabla_segmentos);   //TODO: fijarse si hay que liberar los elementos también
	//free(pcb->archivos_abiertos); //TODO: fijarse si hay que liberar los elementos también
	//list_destroy(pcb->tabla_segmentos);  //TODO: fijarse si hay que liberar los elementos también
	//list_destroy(pcb->archivos_abiertos);//TODO: fijarse si hay que liberar los elementos también
	//TODO: liberar_tabla_segmentos_pcb(pcb->tabla_segmentos);
	//TODO: liberar_archivos_abiertos_pcb(pcb->archivos_abiertos);

	free(pcb);
}
/*
void liberar_tabla_segmentos_pcb(tabla_segmentos) {
	//TODO
}

void liberar_archivos_abiertos_pcb(archivos_abiertos) {
	//TODO
}
*/

void liberar_parametros(char** parametros) {
	for(int i = 0; i < string_array_size(parametros); i++) {
		free(parametros[i]);
	}

	free(parametros);
}

void enviar_pcb(int socket, t_pcb* pcb, t_msj_kernel_cpu op_code, char** parametros_de_instruccion) {
	size_t size_total;
	void* stream_pcb_a_enviar = serializar_pcb(pcb, &size_total, op_code, parametros_de_instruccion);

	send(socket, stream_pcb_a_enviar, size_total, 0);

	free(stream_pcb_a_enviar);
}

void* serializar_pcb(t_pcb* pcb, size_t* size_total, t_msj_kernel_cpu op_code, char** parametros_de_instruccion) {
	size_t size_payload = tamanio_payload_pcb(pcb);
	*size_total = sizeof(t_msj_kernel_cpu);

	if(string_array_size(parametros_de_instruccion)) { //Si hay algun parametro
		*size_total += sizeof(size_t); //Cantidad de parametros
		for(int i = 0; i < string_array_size(parametros_de_instruccion); i++) {
			*size_total += sizeof(size_t) + strlen(parametros_de_instruccion[i]) + 1; //Tamanio de parametro + longitud de parametro
		}
	}

	*size_total += sizeof(size_t) + size_payload;

	void* stream_pcb = malloc(*size_total);
	int desplazamiento = 0;

	//* memcpy código de operación y tamanio de Payload *//

    memcpy(stream_pcb + desplazamiento, &(op_code), sizeof(op_code));
    desplazamiento += sizeof(op_code);

    if(string_array_size(parametros_de_instruccion)) { //Si hay algun parametro
    	size_t cantidad_de_parametros = string_array_size(parametros_de_instruccion);
    	memcpy(stream_pcb + desplazamiento, &(cantidad_de_parametros), sizeof(cantidad_de_parametros));
    	desplazamiento += sizeof(cantidad_de_parametros);

    	size_t size_parametro_de_instruccion;
    	for(int i = 0; i < cantidad_de_parametros; i++) {
    		size_parametro_de_instruccion = strlen(parametros_de_instruccion[i]) + 1;
			memcpy(stream_pcb + desplazamiento, &(size_parametro_de_instruccion), sizeof(size_parametro_de_instruccion));
			desplazamiento += sizeof(size_parametro_de_instruccion);

			memcpy(stream_pcb + desplazamiento, parametros_de_instruccion[i], size_parametro_de_instruccion);
			desplazamiento += size_parametro_de_instruccion;
    	}

    	//string_array_destroy(parametros_de_instruccion);
    } //Para esto no hace falta deserializar en recibir_pcb, ya que esto lo hace la funcion recibir_parametro_de_instruccion

    memcpy(stream_pcb + desplazamiento, &(size_payload), sizeof(size_t));
    desplazamiento += sizeof(size_t);

	//* memcpy Payload *//

	memcpy(stream_pcb + desplazamiento, &(pcb->pid), sizeof(int));
	desplazamiento += sizeof(int);

	int tamanio_instrucciones_en_bytes = tamanio_instrucciones(pcb->instrucciones);
	memcpy(stream_pcb + desplazamiento, &(tamanio_instrucciones_en_bytes), sizeof(int));
	desplazamiento += sizeof(int);

	memcpy_instrucciones_serializar(stream_pcb, pcb->instrucciones, &desplazamiento);

	memcpy(stream_pcb + desplazamiento, &(pcb->pc), sizeof(int));
	desplazamiento += sizeof(int);

	memcpy_registros_serializar(stream_pcb, pcb->registros_cpu, &desplazamiento); // TODO: fijarse si está bien

	//memcpy_tabla_segmentos_serializar(stream_pcb, pcb->tabla_segmentos, &desplazamiento); //TODO: interpretar qué es t_segmento y hacer memcpy

	memcpy(stream_pcb + desplazamiento, &(pcb->estimado_prox_rafaga), sizeof(int));
	desplazamiento += sizeof(int);

	memcpy(stream_pcb + desplazamiento, &(pcb->tiempo_llegada_ready), sizeof(int));
	desplazamiento += sizeof(int);

	//memcpy_archivos_abiertos_serializar(stream_pcb, pcb->archivos_abiertos, &desplazamiento); //TODO: interpretar qué es t_archivo y hacer memcpy

	memcpy(stream_pcb + desplazamiento, &(pcb->socket_consola), sizeof(pcb->socket_consola));
	desplazamiento += sizeof(pcb->socket_consola);

	memcpy(stream_pcb + desplazamiento, &(pcb->tiempo_real_ejecucion), sizeof(pcb->tiempo_real_ejecucion));
	desplazamiento += sizeof(pcb->tiempo_real_ejecucion);

	memcpy(stream_pcb + desplazamiento, &(pcb->tiempo_inicial_ejecucion), sizeof(pcb->tiempo_inicial_ejecucion));
	desplazamiento += sizeof(pcb->tiempo_inicial_ejecucion);

	return stream_pcb;
}

size_t tamanio_payload_pcb(t_pcb* pcb) {
	size_t size = sizeof(pcb->pid) + tamanio_instrucciones(pcb->instrucciones);
	size += sizeof(int); //para el tamanio_instrucciones_en_bytes
	size += sizeof(pcb->pc);
	size += sizeof(char)*16*7; // Por todos los char[] de los registros (?
	size += sizeof(pcb->estimado_prox_rafaga) + sizeof(pcb->tiempo_llegada_ready) + sizeof(pcb->socket_consola);
	size += sizeof(pcb->tiempo_real_ejecucion) + sizeof(pcb->tiempo_inicial_ejecucion);
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

void memcpy_instrucciones_serializar(void* stream_pcb, t_list* instrucciones, int* desplazamiento) {
	for(int i = 0 ; i < instrucciones->elements_count ; i++) {
		t_instruccion* instruccion = (t_instruccion*)list_get(instrucciones, i);
		size_t largo_nombre = strlen(instruccion->nombre)+1;

		memcpy(stream_pcb + (*desplazamiento), &(largo_nombre), sizeof(size_t));		//pongo size de nombre instruccion
		(*desplazamiento) += sizeof(size_t);

		memcpy(stream_pcb + (*desplazamiento), instruccion->nombre, largo_nombre);		//pongo nombre instruccion
		(*desplazamiento) += largo_nombre;

		t_list* parametros = instruccion->parametros;
		for(int j = 0; j < parametros->elements_count; j++) {
			char* parametro = (char*)list_get(parametros, j);
			size_t largo_nombre = strlen(parametro)+1;

			memcpy(stream_pcb + (*desplazamiento), &(largo_nombre), sizeof(size_t));	//pongo size nombre parametro
			(*desplazamiento) += sizeof(size_t);

			memcpy(stream_pcb + (*desplazamiento), parametro, largo_nombre);			//pongo parametro
			(*desplazamiento) += largo_nombre;
		}
	}
}

void memcpy_registros_serializar(void* stream_pcb, t_registros_cpu registros_cpu, int* desplazamiento) {
	memcpy(stream_pcb + *desplazamiento, registros_cpu.AX, sizeof(char) * 4); //Fijarse si se puede hacer pcb->registros_cpu.AX o hay que hacer un strdup antes o algo así.
	*desplazamiento += sizeof(char) * 4;

	memcpy(stream_pcb + *desplazamiento, registros_cpu.BX, sizeof(char) * 4);
	*desplazamiento += sizeof(char) * 4;

	memcpy(stream_pcb + *desplazamiento, registros_cpu.CX, sizeof(char) * 4);
	*desplazamiento += sizeof(char) * 4;

	memcpy(stream_pcb + *desplazamiento, registros_cpu.DX, sizeof(char) * 4);
	*desplazamiento += sizeof(char) * 4;

	memcpy(stream_pcb + *desplazamiento, registros_cpu.EAX, sizeof(char) * 8);
	*desplazamiento += sizeof(char) * 8;

	memcpy(stream_pcb + *desplazamiento, registros_cpu.EBX, sizeof(char) * 8);
	*desplazamiento += sizeof(char) * 8;

	memcpy(stream_pcb + *desplazamiento, registros_cpu.ECX, sizeof(char) * 8);
	*desplazamiento += sizeof(char) * 8;

	memcpy(stream_pcb + *desplazamiento, registros_cpu.EDX, sizeof(char) * 8);
	*desplazamiento += sizeof(char) * 8;

	memcpy(stream_pcb + *desplazamiento, registros_cpu.RAX, sizeof(char) * 16);
	*desplazamiento += sizeof(char) * 16;

	memcpy(stream_pcb + *desplazamiento, registros_cpu.RBX, sizeof(char) * 16);
	*desplazamiento += sizeof(char) * 16;

	memcpy(stream_pcb + *desplazamiento, registros_cpu.RCX, sizeof(char) * 16);
	*desplazamiento += sizeof(char) * 16;

	memcpy(stream_pcb + *desplazamiento, registros_cpu.RDX, sizeof(char) * 16);
	*desplazamiento += sizeof(char) * 16;
}

void memcpy_tabla_segmentos_serializar(void* stream, t_list* tabla_segmentos, int* desplazamiento) {
	for(int i = 0; i < tabla_segmentos->elements_count; i++) {
		//TODO: memcpy
		/*
		t_segmento* segmento = (t_segmento*)list_get(tabla_segmentos, i);
		memcpy(stream + *desplazamiento, segmento, sizeof(t_segmento));
		*desplazamiento += sizeof(t_segmento);
		*/
	}
}

void memcpy_archivos_abiertos_serializar(void* stream, t_list* archivos_abiertos, int* desplazamiento) {
	for(int i = 0; i < archivos_abiertos->elements_count; i++) {
		//TODO: memcpy
		/*
		t_archivo* archivo = (t_archivo*)list_get(archivos_abiertos, i);
		memcpy(stream + *desplazamiento, archivo, sizeof(t_archivo));
		*desplazamiento += sizeof(t_archivo);
		*/
	}
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

    t_pcb* pcb = deserializar_pcb(stream_pcb_a_recibir);

	free(stream_pcb_a_recibir);
	return pcb;
}

t_pcb* deserializar_pcb(void* stream) {
	t_pcb* pcb = (t_pcb*)malloc(sizeof(t_pcb));
	int desplazamiento = 0;

	memcpy(&(pcb->pid), stream + desplazamiento, sizeof(pcb->pid));
	desplazamiento += sizeof(pcb->pid);

	int tamanio_instrucciones_en_bytes;
	memcpy(&(tamanio_instrucciones_en_bytes), stream + desplazamiento, sizeof(int));
	desplazamiento += sizeof(int);

	pcb->instrucciones = deserializar_instrucciones(stream, tamanio_instrucciones_en_bytes + desplazamiento, &desplazamiento);

	//print_l_instrucciones(pcb->instrucciones);

	memcpy(&(pcb->pc), stream + desplazamiento, sizeof(pcb->pc));
	desplazamiento += sizeof(pcb->pc);

	memcpy_registros_deserializar(&(pcb->registros_cpu), stream, &desplazamiento);

	//memcpy_tabla_segmentos_deserializar(pcb->tabla_segmentos, stream, &desplazamiento);

	memcpy(&(pcb->estimado_prox_rafaga), stream + desplazamiento, sizeof(pcb->estimado_prox_rafaga));
	desplazamiento += sizeof(pcb->estimado_prox_rafaga);

	memcpy(&(pcb->tiempo_llegada_ready), stream + desplazamiento, sizeof(pcb->tiempo_llegada_ready));
	desplazamiento += sizeof(pcb->tiempo_llegada_ready);

	//memcpy_archivos_abiertos_deserializar(pcb->archivos_abiertos, stream, &desplazamiento);

	memcpy(&(pcb->socket_consola), stream + desplazamiento, sizeof(pcb->socket_consola));
	desplazamiento += sizeof(pcb->socket_consola);

	memcpy(&(pcb->tiempo_real_ejecucion), stream + desplazamiento, sizeof(pcb->tiempo_real_ejecucion));
	desplazamiento += sizeof(pcb->tiempo_real_ejecucion);

	memcpy(&(pcb->tiempo_inicial_ejecucion), stream + desplazamiento, sizeof(pcb->tiempo_inicial_ejecucion));
	desplazamiento += sizeof(pcb->tiempo_inicial_ejecucion);

	return pcb;
}

t_list* deserializar_instrucciones(void* a_recibir, size_t size_payload, int* desplazamiento) {
    t_list* instrucciones = list_create();
    t_dictionary* diccionario_instrucciones = crear_diccionario_instrucciones();

	while((*desplazamiento) < size_payload) {
    	t_instruccion* instruccion = malloc(sizeof(t_instruccion));
		size_t largo_nombre;

		memcpy(&(largo_nombre), a_recibir + (*desplazamiento), sizeof(size_t));		//pongo size de nombre instruccion
		(*desplazamiento) += sizeof(size_t);
	    //printf("%d\n", (int)largo_nombre);

		instruccion->nombre = malloc(largo_nombre);
		memcpy(instruccion->nombre, a_recibir + (*desplazamiento), largo_nombre);		//pongo nombre instruccion
	    (*desplazamiento) += largo_nombre;

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

		memcpy(&(largo_parametro), a_recibir + (*desplazamiento), sizeof(size_t));		//pongo size de nombre parametro
		(*desplazamiento)+= sizeof(size_t);

		parametro = malloc(largo_parametro);
		memcpy(parametro, a_recibir + (*desplazamiento), largo_parametro);		//pongo nombre parametro
		(*desplazamiento) += largo_parametro;

		list_add(instruccion->parametros, parametro);
    }
}

void memcpy_registros_deserializar(t_registros_cpu* registros_cpu, void* stream_pcb, int* desplazamiento) {
	memcpy(registros_cpu->AX, stream_pcb + *desplazamiento, sizeof(char) * 4); //Fijarse si se puede hacer pcb->registros_cpu.AX o hay que hacer un strdup antes o algo así.
	*desplazamiento += sizeof(char) * 4;

	memcpy(registros_cpu->BX, stream_pcb + *desplazamiento, sizeof(char) * 4);
	*desplazamiento += sizeof(char) * 4;

	memcpy(registros_cpu->CX, stream_pcb + *desplazamiento, sizeof(char) * 4);
	*desplazamiento += sizeof(char) * 4;

	memcpy(registros_cpu->DX, stream_pcb + *desplazamiento, sizeof(char) * 4);
	*desplazamiento += sizeof(char) * 4;

	memcpy(registros_cpu->EAX, stream_pcb + *desplazamiento, sizeof(char) * 8);
	*desplazamiento += sizeof(char) * 8;

	memcpy(registros_cpu->EBX, stream_pcb + *desplazamiento, sizeof(char) * 8);
	*desplazamiento += sizeof(char) * 8;

	memcpy(registros_cpu->ECX, stream_pcb + *desplazamiento, sizeof(char) * 8);
	*desplazamiento += sizeof(char) * 8;

	memcpy(registros_cpu->EDX, stream_pcb + *desplazamiento, sizeof(char) * 8);
	*desplazamiento += sizeof(char) * 8;

	memcpy(registros_cpu->RAX, stream_pcb + *desplazamiento, sizeof(char) * 16);
	*desplazamiento += sizeof(char) * 16;

	memcpy(registros_cpu->RBX, stream_pcb + *desplazamiento, sizeof(char) * 16);
	*desplazamiento += sizeof(char) * 16;

	memcpy(registros_cpu->RCX, stream_pcb + *desplazamiento, sizeof(char) * 16);
	*desplazamiento += sizeof(char) * 16;

	memcpy(registros_cpu->RDX, stream_pcb + *desplazamiento, sizeof(char) * 16);
	*desplazamiento += sizeof(char) * 16;
}

void memcpy_tabla_segmentos_deserializar(t_list* tabla_segmentos, void* stream, int* desplazamiento) {
	for(int i = 0; i < tabla_segmentos->elements_count; i++) {
		//TODO: memcpy
		/*
		t_segmento* segmento = (t_segmento*)list_get(tabla_segmentos, i);
		memcpy(segmento, stream + *desplazamiento, sizeof(t_segmento));
		*desplazamiento += sizeof(t_segmento);
		*/
	}
}

void memcpy_archivos_abiertos_deserializar(t_list* archivos_abiertos, void* stream, int* desplazamiento) {
	for(int i = 0; i < archivos_abiertos->elements_count; i++) {
		//TODO: memcpy
		/*
		t_archivo* archivo = (t_archivo*)list_get(archivos_abiertos, i);
		memcpy(archivo, stream + *desplazamiento, sizeof(t_archivo));
		*desplazamiento += sizeof(t_archivo);
		*/
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

void enviar_msj(int msj,int socket){
	send(socket, &msj, sizeof(int), 0);
}

int recibir_msj(int socket){
	int msj;
	recv(socket, &msj, sizeof(int), MSG_WAITALL);
	return msj;
}

void enviar_msj_con_parametros(int op_code,char** parametros,int socket){
	int size_total;
	size_total = sizeof(int)*2; //op_code y size_payload

		if(string_array_size(parametros)) { //Si hay algun parametro
			for(int i = 0; i < string_array_size(parametros); i++) {
				size_total += sizeof(size_t) + strlen(parametros[i]) + 1; //Tamanio de parametro + longitud de parametro
			}
		}
	size_t size_payload = size_total - 2 * sizeof(int);

	void* stream = malloc(size_total);
	int desplazamiento = 0;

	memcpy(stream + desplazamiento, &(op_code), sizeof(int));
	desplazamiento += sizeof(op_code);

	memcpy(stream + desplazamiento, &size_payload,sizeof(int));
	desplazamiento += sizeof(int);

	if(string_array_size(parametros)) { //Si hay algun parametro
		size_t cantidad_de_parametros = string_array_size(parametros);

		size_t size_parametro_de_instruccion;
		for(int i = 0; i < cantidad_de_parametros; i++) {
			size_parametro_de_instruccion = strlen(parametros[i]) + 1;
			memcpy(stream + desplazamiento, &(size_parametro_de_instruccion), sizeof(size_parametro_de_instruccion));
			desplazamiento += sizeof(size_parametro_de_instruccion);

			memcpy(stream + desplazamiento, parametros[i], size_parametro_de_instruccion);
			desplazamiento += size_parametro_de_instruccion;
		}
	}

	send(socket, stream, size_total, 0);
}
