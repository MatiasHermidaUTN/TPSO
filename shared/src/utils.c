#include "utils.h"

/////////////
// Cliente //
/////////////

int crear_conexion(char* ip, char* puerto) {
	struct addrinfo hints;
	struct addrinfo* server_info;

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

//////////////
// Servidor //
//////////////

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
		printf("No se pudo crear un logger.\n");
		exit(EXIT_FAILURE);
	}

	return nuevo_logger;
}

//TODO: eliminar, ya está recibir_msj()
t_handshake recibir_handshake(int socket_cliente) {
	t_handshake respuesta_handshake;
	recv(socket_cliente, &respuesta_handshake, sizeof(respuesta_handshake), MSG_WAITALL);
	return respuesta_handshake;
}

//TODO: eliminar, ya está enviar_msj()
void enviar_handshake(int socket, t_handshake mensaje_handshake) {
	//TODO: Falta fijarse si da error
	send(socket, &mensaje_handshake, sizeof(mensaje_handshake), 0);
}

//////////////////////
// Listas de estado //
//////////////////////

void* queue_pop_con_mutex(t_queue* queue, pthread_mutex_t* mutex) {
    pthread_mutex_lock(mutex);
    void* elemento = queue_pop(queue);
    pthread_mutex_unlock(mutex);
    return elemento;
}

void queue_push_con_mutex(t_queue* queue, void* elemento, pthread_mutex_t* mutex) {
    pthread_mutex_lock(mutex);
    queue_push(queue, elemento);
    pthread_mutex_unlock(mutex);
    return;
}

void* list_pop_con_mutex(t_list* lista, pthread_mutex_t* mutex) {
    pthread_mutex_lock(mutex);
    void* elemento = list_remove(lista, 0);
    pthread_mutex_unlock(mutex);
    return elemento;
}

void list_push_con_mutex(t_list* lista, void* elemento, pthread_mutex_t* mutex) {
    pthread_mutex_lock(mutex);
    list_add(lista, elemento);
    pthread_mutex_unlock(mutex);
    return;
}

///////////////////
// Liberar datos //
///////////////////

void liberar_pcb(t_pcb* pcb) {
	list_destroy_and_destroy_elements(pcb->instrucciones, (void*)destruir_instruccion);
	liberar_tabla_segmentos(pcb->tabla_segmentos);
	list_destroy_and_destroy_elements(pcb->archivos_abiertos, (void*)destruir_archivo_abierto);
	list_destroy_and_destroy_elements(pcb->recursos, (void*)free);
	free(pcb);
}

void destruir_instruccion(t_instruccion* instruccion) {
	free(instruccion->nombre);
	list_destroy_and_destroy_elements(instruccion->parametros, (void*)free);
	free(instruccion);
}

void destruir_archivo_abierto(t_archivo_abierto* archivo){
	free(archivo->nombre_archivo);
	free(archivo);
}

void liberar_tabla_segmentos(t_list* tabla_segmentos) {
	list_destroy_and_destroy_elements(tabla_segmentos, (void*)free);
}

void liberar_parametros(char** parametros) {
	for(int i = 0; i < string_array_size(parametros); i++) {
		free(parametros[i]);
	}

	free(parametros);
}

/////////
// pcb //
/////////

void enviar_pcb(int socket, t_pcb* pcb, t_msj_kernel_cpu op_code, char** parametros_de_instruccion) {
	size_t size_total;

	void* stream_pcb_a_enviar = serializar_pcb(pcb, &size_total, op_code, parametros_de_instruccion);

	send(socket, stream_pcb_a_enviar, size_total, 0);

	free(stream_pcb_a_enviar);
}

void* serializar_pcb(t_pcb* pcb, size_t* size_total, t_msj_kernel_cpu op_code, char** parametros_de_instruccion) {
	size_t size_payload = tamanio_payload_pcb(pcb);
	*size_total = sizeof(t_msj_kernel_cpu);

	*size_total += sizeof(size_t); //Cantidad de parametros
	for(int i = 0; i < string_array_size(parametros_de_instruccion); i++) {
		*size_total += sizeof(size_t) + strlen(parametros_de_instruccion[i]) + 1; //Tamanio de parametro + longitud de parametro
	}

	*size_total += sizeof(size_t) + size_payload;

	void* stream_pcb = malloc(*size_total);
	int desplazamiento = 0;

	////////////////////////////////
	// memcpy código de operación //
	////////////////////////////////

    memcpy(stream_pcb + desplazamiento, &op_code, sizeof(op_code));
    desplazamiento += sizeof(op_code);

    ///////////////////////
    // memcpy parámetros //
    ///////////////////////

    //Para esto no hace falta deserializar en recibir_pcb, ya que esto lo hace la funcion recibir_parametro_de_instruccion

    size_t cantidad_de_parametros = string_array_size(parametros_de_instruccion);
    memcpy(stream_pcb + desplazamiento, &cantidad_de_parametros, sizeof(cantidad_de_parametros));
    desplazamiento += sizeof(cantidad_de_parametros);

   	size_t size_parametro_de_instruccion;
   	for(int i = 0; i < cantidad_de_parametros; i++) {
   		size_parametro_de_instruccion = strlen(parametros_de_instruccion[i]) + 1;
		memcpy(stream_pcb + desplazamiento, &size_parametro_de_instruccion, sizeof(size_parametro_de_instruccion));
		desplazamiento += sizeof(size_parametro_de_instruccion);

		memcpy(stream_pcb + desplazamiento, parametros_de_instruccion[i], size_parametro_de_instruccion);
		desplazamiento += size_parametro_de_instruccion;
   	}

   	//string_array_destroy(parametros_de_instruccion);

   	///////////////////////////////
   	// memcpy tamanio de Payload //
   	///////////////////////////////

    memcpy(stream_pcb + desplazamiento, &(size_payload), sizeof(size_payload));
    desplazamiento += sizeof(size_payload);

    ////////////////////
	// memcpy Payload //
    ////////////////////

	memcpy(stream_pcb + desplazamiento, &(pcb->pid), sizeof(pcb->pid));
	desplazamiento += sizeof(pcb->pid);

	int tamanio_instrucciones_en_bytes = tamanio_instrucciones(pcb->instrucciones);
	memcpy(stream_pcb + desplazamiento, &(tamanio_instrucciones_en_bytes), sizeof(tamanio_instrucciones_en_bytes));
	desplazamiento += sizeof(tamanio_instrucciones_en_bytes);

	memcpy_instrucciones_serializar(stream_pcb, pcb->instrucciones, &desplazamiento);

	memcpy(stream_pcb + desplazamiento, &(pcb->pc), sizeof(int));
	desplazamiento += sizeof(int);

	memcpy_registros_serializar(stream_pcb, pcb->registros_cpu, &desplazamiento);

	memcpy_tabla_segmentos_serializar(stream_pcb, pcb->tabla_segmentos, &desplazamiento);

	memcpy(stream_pcb + desplazamiento, &(pcb->estimado_prox_rafaga), sizeof(pcb->estimado_prox_rafaga));
	desplazamiento += sizeof(pcb->estimado_prox_rafaga);

	memcpy(stream_pcb + desplazamiento, &(pcb->tiempo_llegada_ready), sizeof(pcb->tiempo_llegada_ready));
	desplazamiento += sizeof(pcb->tiempo_llegada_ready);

	memcpy_archivos_abiertos_serializar(stream_pcb, pcb->archivos_abiertos, &desplazamiento);

	memcpy_recursos_serializar(stream_pcb, pcb->recursos, &desplazamiento);

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
	size += sizeof(char) * 16 * 7; // Por todos los char[] de los registros (?

	size += sizeof(int); //cantidad de segmentos
	size += list_size(pcb->tabla_segmentos) * sizeof(int) * 3; //*3 por id + dir_base + tamanio

	size += sizeof(pcb->estimado_prox_rafaga) + sizeof(pcb->tiempo_llegada_ready) + sizeof(pcb->socket_consola);
	size += sizeof(pcb->tiempo_real_ejecucion) + sizeof(pcb->tiempo_inicial_ejecucion);

	size += sizeof(int); // cantidad de archivos abiertos
	for(int i = 0; i < list_size(pcb->archivos_abiertos); i++) {
		t_archivo_abierto* archivo_abierto =  list_get(pcb->archivos_abiertos, i);
		size += sizeof(int) + sizeof(int) + strlen(archivo_abierto->nombre_archivo) + 1; //posicion actual + tamanio nombre + nombre del archivo
	}

	size += sizeof(int); // cantidad de recursos
	for(int i = 0; i < list_size(pcb->recursos); i++) {
		char* nombre_recurso =  list_get(pcb->recursos, i);
		size += sizeof(int) + strlen(nombre_recurso) + 1; //tamanio nombre + nombre del archivo
	}

	return size;
}

size_t tamanio_instrucciones(t_list* instrucciones) {
	int size = 0;
	for(int i = 0; i < list_size(instrucciones); i++) {
		t_instruccion* instruccion = list_get(instrucciones, i);
	    size += sizeof(size_t)	//para longitud del nombre / para decir cuantos char son el nombre de instruccion
	    		+ strlen(instruccion->nombre) + 1
				+ tamanio_parametros(instruccion->parametros, i);
    }
	return size;
}

int tamanio_parametros(t_list* parametros, int index_instruccion) {
	int size_parametro = 0;
	for(int i = 0; i < list_size(parametros); i++) {
	    size_parametro += sizeof(size_t)	//para decir cuantos char son el nombre de parametro
	    		+ strlen(list_get(parametros, i)) + 1;
    }
	return size_parametro;
}

void memcpy_instrucciones_serializar(void* stream_pcb, t_list* instrucciones, int* desplazamiento) {
	for(int i = 0 ; i < list_size(instrucciones); i++) {
		t_instruccion* instruccion = list_get(instrucciones, i);
		size_t largo_nombre = strlen(instruccion->nombre)+1;

		memcpy(stream_pcb + (*desplazamiento), &(largo_nombre), sizeof(size_t));		//pongo size de nombre instruccion
		(*desplazamiento) += sizeof(size_t);

		memcpy(stream_pcb + (*desplazamiento), instruccion->nombre, largo_nombre);		//pongo nombre instruccion
		(*desplazamiento) += largo_nombre;

		t_list* parametros = instruccion->parametros;
		for(int j = 0; j < list_size(parametros); j++) {
			char* parametro = list_get(parametros, j);
			size_t largo_nombre = strlen(parametro) + 1;

			memcpy(stream_pcb + *desplazamiento, &largo_nombre, sizeof(size_t));	//pongo size nombre parametro
			*desplazamiento += sizeof(size_t);

			memcpy(stream_pcb + *desplazamiento, parametro, largo_nombre);			//pongo parametro
			*desplazamiento += largo_nombre;
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
	int cantidad_segmentos = list_size(tabla_segmentos);
	//printf("CANTIDAD_SEGMENTOS: %d\n", cantidad_segmentos);
	memcpy(stream + *desplazamiento, &cantidad_segmentos, sizeof(int));
	*desplazamiento += sizeof(int);

	for(int i = 0; i < cantidad_segmentos; i++) {
		t_segmento* segmento = list_get(tabla_segmentos, i);
		memcpy(stream + *desplazamiento, &(segmento->id), sizeof(int));
		*desplazamiento += sizeof(int);

		memcpy(stream + *desplazamiento, &(segmento->direccion_base), sizeof(int));
		*desplazamiento += sizeof(int);

		memcpy(stream + *desplazamiento, &(segmento->tamanio), sizeof(int));
		*desplazamiento += sizeof(int);
	}
}

void memcpy_archivos_abiertos_serializar(void* stream, t_list* archivos_abiertos, int* desplazamiento) {
	int tamanio_lista = list_size(archivos_abiertos);
	memcpy(stream + *desplazamiento, &tamanio_lista, sizeof(int));
	*desplazamiento += sizeof(int);

	for(int i = 0; i < tamanio_lista; i++) {
		t_archivo_abierto* archivo_abierto = list_get(archivos_abiertos, i);

		memcpy(stream + *desplazamiento, &(archivo_abierto->posicion_actual), sizeof(int));
		*desplazamiento += sizeof(int);

		int tamanio_nombre = strlen(archivo_abierto->nombre_archivo) + 1;
		memcpy(stream + *desplazamiento, &tamanio_nombre, sizeof(int));
		*desplazamiento += sizeof(int);

		memcpy(stream + *desplazamiento, archivo_abierto->nombre_archivo, tamanio_nombre);
		*desplazamiento += tamanio_nombre;
	}
}

void memcpy_recursos_serializar(void* stream, t_list* recursos, int* desplazamiento) {
	int tamanio_lista = list_size(recursos);
	memcpy(stream + *desplazamiento, &tamanio_lista, sizeof(int));
	*desplazamiento += sizeof(int);

	for(int i = 0; i < tamanio_lista; i++) {
		char* nombre_recurso = list_get(recursos, i);

		int tamanio_nombre = strlen(nombre_recurso) + 1;
		memcpy(stream + *desplazamiento, &tamanio_nombre, sizeof(int));
		*desplazamiento += sizeof(int);

		memcpy(stream + *desplazamiento, nombre_recurso, tamanio_nombre);
		*desplazamiento += tamanio_nombre;
	}
}

t_pcb* recibir_pcb(int socket) {
	size_t size_payload;
    if (recv(socket, &size_payload, sizeof(size_t), 0) != sizeof(size_t)) {
        exit(EXIT_FAILURE);
    }

	void* stream_pcb_a_recibir = malloc(size_payload);
    if (recv(socket, stream_pcb_a_recibir, size_payload, 0) != size_payload) {
        free(stream_pcb_a_recibir);
        exit(EXIT_FAILURE);
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

	memcpy(&(pcb->pc), stream + desplazamiento, sizeof(pcb->pc));
	desplazamiento += sizeof(pcb->pc);

	memcpy_registros_deserializar(&(pcb->registros_cpu), stream, &desplazamiento);

	memcpy_tabla_segmentos_deserializar(pcb, stream, &desplazamiento);

	memcpy(&(pcb->estimado_prox_rafaga), stream + desplazamiento, sizeof(pcb->estimado_prox_rafaga));
	desplazamiento += sizeof(pcb->estimado_prox_rafaga);

	memcpy(&(pcb->tiempo_llegada_ready), stream + desplazamiento, sizeof(pcb->tiempo_llegada_ready));
	desplazamiento += sizeof(pcb->tiempo_llegada_ready);

	memcpy_archivos_abiertos_deserializar(pcb, stream, &desplazamiento);

	memcpy_recursos_deserializar(pcb, stream, &desplazamiento);

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

	while(*desplazamiento < size_payload) {
    	t_instruccion* instruccion = malloc(sizeof(t_instruccion));
		size_t largo_nombre;

		memcpy(&largo_nombre, a_recibir + *desplazamiento, sizeof(size_t));
		*desplazamiento += sizeof(size_t);

		instruccion->nombre = malloc(largo_nombre);
		memcpy(instruccion->nombre, a_recibir + *desplazamiento, largo_nombre);
	    *desplazamiento += largo_nombre;

		instruccion->parametros = list_create();

		deserializar_parametros(a_recibir, desplazamiento, instruccion);

		list_add(instrucciones, instruccion);
	}

    return instrucciones;
}

void deserializar_parametros(void* a_recibir, int* desplazamiento, t_instruccion* instruccion) {
    t_dictionary* diccionario_instrucciones = crear_diccionario_instrucciones();

	if(!dictionary_has_key(diccionario_instrucciones, instruccion->nombre)) {
		printf("No existe instruccion: %s , en el diccionario", instruccion->nombre);
		exit(EXIT_FAILURE);
	}

	int* cantidad_de_parametros_esperados = dictionary_get(diccionario_instrucciones, instruccion->nombre);

	for(int j = 0; j < *cantidad_de_parametros_esperados; j++) {
		size_t largo_parametro;
		memcpy(&largo_parametro, a_recibir + *desplazamiento, sizeof(size_t));
		*desplazamiento += sizeof(size_t);

		char* parametro = malloc(largo_parametro);
		memcpy(parametro, a_recibir + *desplazamiento, largo_parametro);
		*desplazamiento += largo_parametro;

		list_add(instruccion->parametros, parametro);
    }

    dictionary_destroy_and_destroy_elements(diccionario_instrucciones, (void*)free);
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

void memcpy_tabla_segmentos_deserializar(t_pcb* pcb, void* stream, int* desplazamiento) {
	int cantidad_segmentos;
	memcpy(&cantidad_segmentos, stream + *desplazamiento, sizeof(int));
	*desplazamiento += sizeof(int);

	pcb->tabla_segmentos = list_create();

	for(int i = 0; i < cantidad_segmentos; i++) {
		t_segmento* segmento = malloc(sizeof(t_segmento));
		memcpy(&(segmento->id), stream + *desplazamiento, sizeof(int));
		*desplazamiento += sizeof(int);

		memcpy(&(segmento->direccion_base), stream + *desplazamiento, sizeof(int));
		*desplazamiento += sizeof(int);

		memcpy(&(segmento->tamanio), stream + *desplazamiento, sizeof(int));
		*desplazamiento += sizeof(int);

		list_add(pcb->tabla_segmentos, segmento);
	}
}

void memcpy_archivos_abiertos_deserializar(t_pcb* pcb, void* stream, int* desplazamiento) {
	int cantidad_archivos_abiertos;
	pcb->archivos_abiertos = list_create();

	memcpy(&cantidad_archivos_abiertos, stream + *desplazamiento, sizeof(int));
	*desplazamiento += sizeof(int);

	for(int i = 0; i < cantidad_archivos_abiertos; i++) {
		t_archivo_abierto* archivo_abierto = malloc(sizeof(t_archivo_abierto));
		memcpy(&(archivo_abierto->posicion_actual), stream + *desplazamiento, sizeof(int));
		*desplazamiento += sizeof(int);

		int tamanio_nombre;
		memcpy(&tamanio_nombre, stream + *desplazamiento, sizeof(int));
		*desplazamiento += sizeof(int);

		archivo_abierto->nombre_archivo = malloc(tamanio_nombre);
		memcpy(archivo_abierto->nombre_archivo, stream + *desplazamiento, tamanio_nombre);
		*desplazamiento += tamanio_nombre;

		list_add(pcb->archivos_abiertos, archivo_abierto);
	}
}

void memcpy_recursos_deserializar(t_pcb* pcb, void* stream, int* desplazamiento) {
	int cantidad_recursos;
	pcb->recursos = list_create();

	memcpy(&cantidad_recursos, stream + *desplazamiento, sizeof(int));
	*desplazamiento += sizeof(int);

	for(int i = 0; i < cantidad_recursos; i++) {
		int tamanio_nombre;
		memcpy(&tamanio_nombre, stream + *desplazamiento, sizeof(int));
		*desplazamiento += sizeof(int);

		char* nombre_recurso = malloc(tamanio_nombre);
		memcpy(nombre_recurso, stream + *desplazamiento, tamanio_nombre);
		*desplazamiento += tamanio_nombre;

		list_add(pcb->recursos, nombre_recurso);
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

//////////////
// Mensajes //
//////////////

void enviar_msj(int socket, int msj) {
	send(socket, &msj, sizeof(msj), 0);
}

int recibir_msj(int socket_cliente) {
	int msj;
	if(recv(socket_cliente, &msj, sizeof(msj), MSG_WAITALL) > 0) {
		return msj;
	}
	else {
		close(socket_cliente);
		return -1;
	}
}

void enviar_msj_con_parametros(int socket, int op_code, char** parametros) {
	int size_payload = 0;
	int size_total = sizeof(op_code) + sizeof(size_payload);

	for(int i = 0; i < string_array_size(parametros); i++) {
		size_payload += sizeof(int) + strlen(parametros[i]) + 1; //Tamanio de parametro + longitud de parametro
	}
	size_total += size_payload;

	void* stream = malloc(size_total);
	int desplazamiento = 0;

	memcpy(stream + desplazamiento, &(op_code), sizeof(op_code));
	desplazamiento += sizeof(op_code);

	memcpy(stream + desplazamiento, &size_payload, sizeof(size_payload));
	desplazamiento += sizeof(size_payload);

	int size_parametro_de_instruccion;
	for(int i = 0; i < string_array_size(parametros); i++) {
		size_parametro_de_instruccion = strlen(parametros[i]) + 1;
		memcpy(stream + desplazamiento, &(size_parametro_de_instruccion), sizeof(size_parametro_de_instruccion));
		desplazamiento += sizeof(size_parametro_de_instruccion);

		memcpy(stream + desplazamiento, parametros[i], size_parametro_de_instruccion);
		desplazamiento += size_parametro_de_instruccion;
	}

	send(socket, stream, size_total, 0);

	free(stream);
}

char** recibir_parametros_de_mensaje(int socket) {
	int desplazamiento = 0;

	int tamanio_payload = 0;
	recv(socket, &tamanio_payload, sizeof(int), MSG_WAITALL);

	void* stream = malloc(tamanio_payload);
	recv(socket, stream, tamanio_payload, MSG_WAITALL);

	char ** parametros = string_array_new();

	while(desplazamiento < tamanio_payload) {
		int tamanio_parametros;
		memcpy(&tamanio_parametros, stream + desplazamiento, sizeof(int));
		desplazamiento += sizeof(int);

		char* parametro = malloc(tamanio_parametros);
		memcpy(parametro, stream + desplazamiento, tamanio_parametros);
		desplazamiento += tamanio_parametros;

		string_array_push(&parametros, parametro);
	}

	free(stream);
	return parametros;
}

///////////////////////////
// Segmentos actualizdos //
///////////////////////////

void enviar_tabla_segmentos(int socket, t_list* tabla_segmentos, t_msj_memoria mensaje) {
	size_t size_total;
	void* stream = serializar_tabla_segmentos(tabla_segmentos, mensaje, &size_total);

	send(socket, stream, size_total, 0);

	free(stream);
	list_destroy_and_destroy_elements(tabla_segmentos, (void*)free);
}

void* serializar_tabla_segmentos(t_list* tabla_segmentos, t_msj_memoria mensaje, size_t* size_total) {
	size_t size_payload = 0;
	*size_total = sizeof(mensaje) + sizeof(size_payload);

	size_payload += list_size(tabla_segmentos) * sizeof(int) * 3; //id + direccion_base + tamanio

	*size_total += size_payload;

	void* stream = malloc(*size_total);
	int desplazamiento = 0;

	memcpy(stream + desplazamiento, &(mensaje), sizeof(mensaje));
	desplazamiento += sizeof(mensaje);

	memcpy(stream + desplazamiento, &size_payload, sizeof(size_payload));
	desplazamiento += sizeof(size_payload);

	t_segmento* segmento;

	for(int i = 0; i < list_size(tabla_segmentos); i++) {
		segmento = list_get(tabla_segmentos, i);

		memcpy(stream + desplazamiento, &(segmento->id), sizeof(segmento->id));
		desplazamiento += sizeof(segmento->id);

		memcpy(stream + desplazamiento, &(segmento->direccion_base), sizeof(segmento->direccion_base));
		desplazamiento += sizeof(segmento->direccion_base);

		memcpy(stream + desplazamiento, &(segmento->tamanio), sizeof(segmento->tamanio));
		desplazamiento += sizeof(segmento->tamanio);
	}

	return stream;
}

t_list* recibir_tabla_segmentos(int socket) {
	size_t size_payload;
    if (recv(socket, &size_payload, sizeof(size_t), 0) != sizeof(size_t)) {
        exit(EXIT_FAILURE);
    }

	void* stream_a_recibir = malloc(size_payload);
    if (recv(socket, stream_a_recibir, size_payload, 0) != size_payload) {
        free(stream_a_recibir);
        exit(EXIT_FAILURE);
    }

    t_list* tabla_segmentos = deserializar_tabla_segmentos(stream_a_recibir, size_payload);

	free(stream_a_recibir);
	return tabla_segmentos;
}

t_list* deserializar_tabla_segmentos(void* stream, int size_payload) {
	size_t desplazamiento = 0;

	t_list* tabla_segmentos = list_create();

	t_segmento* segmento;

	while(desplazamiento < size_payload) {
		segmento = malloc(sizeof(t_segmento));

		memcpy(&(segmento->id), stream + desplazamiento, sizeof(segmento->id));
		desplazamiento += sizeof(segmento->id);

		memcpy(&(segmento->direccion_base), stream + desplazamiento, sizeof(segmento->direccion_base));
		desplazamiento += sizeof(segmento->direccion_base);

		memcpy(&(segmento->tamanio), stream + desplazamiento, sizeof(segmento->tamanio));
		desplazamiento += sizeof(segmento->tamanio);

		list_add(tabla_segmentos, segmento);
	}

	return tabla_segmentos;
}

void enviar_procesos_con_segmentos(int socket, t_list* procesos_actualizados) {
	size_t size_total;
	void* stream = serializar_procesos_con_segmentos(procesos_actualizados, &size_total);

	send(socket, stream, size_total, 0);

	free(stream);
	list_destroy_and_destroy_elements(procesos_actualizados, (void*)liberar_tabla_segmentos);
}

void* serializar_procesos_con_segmentos(t_list* procesos_actualizados, size_t* size_total) {
	size_t size_payload = 0;
	*size_total = sizeof(t_msj_memoria) + sizeof(size_payload);

	t_proceso_actualizado* proceso;
	for(int i = 0; i < list_size(procesos_actualizados); i++) {
		size_payload += sizeof(int) * 2; //pid + cantidad_segmentos

		proceso = list_get(procesos_actualizados, i);
		size_payload += list_size(proceso->tabla_segmentos) * sizeof(int) * 3; //id + direccion_base + tamanio
	}

	*size_total += size_payload;

	void* stream = malloc(*size_total);
	int desplazamiento = 0;

	t_msj_memoria op_code = MEMORIA_COMPACTADA;
	memcpy(stream + desplazamiento, &(op_code), sizeof(op_code));
	desplazamiento += sizeof(op_code);

	memcpy(stream + desplazamiento, &size_payload, sizeof(size_payload));
	desplazamiento += sizeof(size_payload);

	t_segmento* segmento;
	int cantidad_segmentos;
	for(int i = 0; i < list_size(procesos_actualizados); i++) {
		proceso = list_get(procesos_actualizados, i);
		memcpy(stream + desplazamiento, &(proceso->pid), sizeof(proceso->pid));
		desplazamiento += sizeof(proceso->pid);

		cantidad_segmentos = list_size(proceso->tabla_segmentos);
		memcpy(stream + desplazamiento, &cantidad_segmentos, sizeof(cantidad_segmentos));
		desplazamiento += sizeof(cantidad_segmentos);

		for(int j = 0; j < cantidad_segmentos; j++) {
			segmento = list_get(proceso->tabla_segmentos, j);

			memcpy(stream + desplazamiento, &(segmento->id), sizeof(segmento->id));
			desplazamiento += sizeof(segmento->id);

			memcpy(stream + desplazamiento, &(segmento->direccion_base), sizeof(segmento->direccion_base));
			desplazamiento += sizeof(segmento->direccion_base);

			memcpy(stream + desplazamiento, &(segmento->tamanio), sizeof(segmento->tamanio));
			desplazamiento += sizeof(segmento->tamanio);
		}
	}

	return stream;
}

t_list* recibir_procesos_con_segmentos(int socket) {
	size_t size_payload;
    if (recv(socket, &size_payload, sizeof(size_t), 0) != sizeof(size_t)) {
        exit(EXIT_FAILURE);
    }

	void* stream_a_recibir = malloc(size_payload);
    if (recv(socket, stream_a_recibir, size_payload, 0) != size_payload) {
        free(stream_a_recibir);
        exit(EXIT_FAILURE);
    }

    t_list* procesos = deserializar_procesos_con_segmentos(stream_a_recibir,  size_payload);

	free(stream_a_recibir);
	return procesos;
}

t_list* deserializar_procesos_con_segmentos(void* stream, size_t size_payload) {
	size_t desplazamiento = 0;


	t_list* procesos_actualizados = list_create();

	t_proceso_actualizado* proceso;
	t_segmento* segmento;
	int cantidad_segmentos;

	while(desplazamiento < size_payload) {
		proceso = malloc(sizeof(t_proceso_actualizado));

		memcpy(&(proceso->pid), stream + desplazamiento, sizeof(proceso->pid));
		desplazamiento += sizeof(proceso->pid);

		memcpy(&cantidad_segmentos, stream + desplazamiento, sizeof(cantidad_segmentos));
		desplazamiento += sizeof(cantidad_segmentos);

		proceso->tabla_segmentos = list_create();
		for(int j = 0; j < cantidad_segmentos; j++) {
			segmento = malloc(sizeof(t_segmento));

			memcpy(&(segmento->id), stream + desplazamiento, sizeof(segmento->id));
			desplazamiento += sizeof(segmento->id);

			memcpy(&(segmento->direccion_base), stream + desplazamiento, sizeof(segmento->direccion_base));
			desplazamiento += sizeof(segmento->direccion_base);

			memcpy(&(segmento->tamanio), stream + desplazamiento, sizeof(segmento->tamanio));
			desplazamiento += sizeof(segmento->tamanio);

			list_add(proceso->tabla_segmentos, segmento);
		}

		list_add(procesos_actualizados, proceso);
	}

	return procesos_actualizados;
}




