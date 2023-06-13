#include "../include/memoria.h"

t_list* lista_procesos;
void* memoria_principal;
void* bitmap_pointer;
t_bitarray* bitarray_de_bitmap;

int main(int argc, char** argv) {
	t_config* config = iniciar_config(argv[1]);
	leer_memoria_config(config);
    logger = iniciar_logger("memoria.log", "Memoria");
	logger_no_obligatorio = iniciar_logger("memoria_logs_no_obligatorios.log", "Memoria");

    lista_fifo_msj = list_create();
	lista_procesos = list_create();
	memoria_principal = malloc(atoi(lectura_de_config.TAM_MEMORIA)); //puntero a memoria principal

	memset(memoria_principal, 0, atoi(lectura_de_config.TAM_MEMORIA)); //seteo memoria en 0

	//INICIO SEMAFORO FIFO de MENSAJES DE KERNEL, CPU y FS
	sem_init(&sem_cant_msj, 0, 0); 		//msj es tarea a ejecutar(escribir, leer, etc.)
	pthread_mutex_init(&mutex_cola_msj, NULL);

	//CREO SERVIDOR Y ESPERO CLIENTES
    socket_memoria = iniciar_servidor("127.0.0.1", lectura_de_config.PUERTO_ESCUCHA); 
	log_warning(logger_no_obligatorio, "Memoria lista para recibir al cliente");
	for(int i = 0 ; i<3 ; ){		//3 pq hay que aceptar 1 de cada: Kernel, CPU y FileSystem
		i += recibir_conexiones(socket_memoria, logger); //Recibe conexiones (es bloqueante) y crea hilos para manejarlas, devuelve 1 con cada conexion recibida
	}

	//BITMAP
	int tamanio_bitmap = (int)(atoi(lectura_de_config.TAM_MEMORIA) / 8);
	bitmap_pointer = malloc(tamanio_bitmap); 
	if((atoi(lectura_de_config.TAM_MEMORIA) % 8)){ //error por si el bitmap no tiene el tamaño correcto
		log_error(logger_no_obligatorio, "El tamaño de la memoria no es multiplo de 8");
	}
	bitarray_de_bitmap = bitarray_create_with_mode(bitmap_pointer, tamanio_bitmap, MSB_FIRST);

	//Segmento 0
	for(int i = 0; i < atoi(lectura_de_config.TAM_SEGMENTO_0); i++){
		bitarray_set_bit(bitarray_de_bitmap, i);
	}

	while(manejar_mensaje());

	log_destroy(logger);
	log_destroy(logger_no_obligatorio);
	config_destroy(config);
	liberar_estructura_config();
	free(memoria_principal);	
	bitarray_destroy(bitarray_de_bitmap);
	free(bitmap_pointer);
	eliminar_lista_procesos();
	eliminar_lista_mensajes();
	pthread_mutex_destroy(&mutex_cola_msj);
	sem_destroy(&sem_cant_msj);
	close(socket_memoria);
	//TODO eliminar los hilos escuchadores, por ahi no hace falta

	return EXIT_SUCCESS;
}

int manejar_mensaje() { //pinta bien
	sem_wait(&sem_cant_msj);

	t_mensajes* mensaje = list_pop_con_mutex(lista_fifo_msj, &mutex_cola_msj);

	int pid;
	int id_segmento;
	int tamanio_segmento;
	int dir_fisica;
	int tamanio_buffer;
	char* buffer;

	int base;

	switch (mensaje->cod_op) {
		case INICIALIZAR_PROCESO:{
			//-----ENTRADA-----//
			pid = atoi(mensaje->parametros[0]);
			//-----------------//

			nodoProceso* nodoP = crear_proceso(pid);

			log_info(logger, "Creación de Proceso PID: %d", pid); //log obligatorio

			//-----SALIDA-----//			
			enviar_tabla_segmentos_memoria(socket_kernel, nodoP->lista_segmentos, PROCESO_INICIALIZADO);
			//----------------//

			break;
		}
		case CREAR_SEGMENTO:{
			//-----ENTRADA-----//
			id_segmento = atoi(mensaje->parametros[0]);
			tamanio_segmento = atoi(mensaje->parametros[1]);
			pid = atoi(mensaje->parametros[2]);			
			//-----------------//

			if(!tengo_espacio_general(tamanio_segmento)) {
				
				log_error(logger_no_obligatorio, "no hay espacio para segmento");
				enviar_msj(socket_kernel, NO_HAY_ESPACIO_DISPONIBLE);
				break;
			}
			if(!tengo_espacio_contiguo(tamanio_segmento)) {
				
				log_warning(logger_no_obligatorio, "hay que compactar");
				enviar_msj(socket_kernel, HAY_QUE_COMPACTAR); 
				break;
			}
			base = crear_segmento(pid, id_segmento, tamanio_segmento); 

			log_info(logger, "PID: %d - Crear Segmento: %d - Base: %d - TAMAÑO: %d", pid, id_segmento, base, tamanio_segmento); //log obligatorio			

			//-----SALIDA-----//	
			nodoProceso* nodoP = buscar_por_pid(pid);

			enviar_tabla_segmentos_memoria(socket_kernel, nodoP->lista_segmentos, SEGMENTO_CREADO);
			//----------------//

			break;
		}
		case ELIMINAR_SEGMENTO:{
			//-----ENTRADA-----//
			id_segmento = atoi(mensaje->parametros[0]);
			pid = atoi(mensaje->parametros[1]);			
			//-----------------//

			nodoProceso* nodoP = buscar_por_pid(pid);
			nodoSegmento* nodoS = buscar_por_id(nodoP->lista_segmentos, id_segmento);

			base = nodoS->base;
			int tamanio_segmento = nodoS->tamanio;
			
			eliminar_segmento(pid, id_segmento);

			log_info(logger, "PID: %d - Eliminar Segmento: %d - Base: %d - TAMAÑO: %d", pid, id_segmento, base, tamanio_segmento); //log obligatorio

			//-----SALIDA-----//	
			enviar_tabla_segmentos_memoria(socket_kernel, nodoP->lista_segmentos, SEGMENTO_ELIMINADO);
			//----------------//			

			break;
		}
		case ELIMINAR_PROCESO:{
			//-----ENTRADA-----//
			pid = atoi(mensaje->parametros[0]);
			//-----------------//

			eliminar_proceso(pid);

			log_info(logger, "Eliminación de Proceso PID: %d", pid); //log obligatorio
			
			//-----SALIDA-----//	
			//enviar_msj(socket_kernel, PROCESO_ELIMINADO); Parece q no se necesita (decision de kernel)
			//----------------//

			break;
		}
		case ESCRIBIR_VALOR:{
			//-----ENTRADA-----//
			dir_fisica = atoi(mensaje->parametros[0]);
			buffer = mensaje->parametros[1];

			tamanio_buffer = string_length(buffer); //Saca el /0 (eso es bueno)
			//-----------------//

			buscar_pid_y_id_segmento_por_dir_fisica(dir_fisica, &pid, &id_segmento);

			hay_seg_fault(pid, id_segmento, dir_fisica, tamanio_buffer);

			memcpy(memoria_principal + dir_fisica, buffer, tamanio_buffer);

			char* origen_mensaje_string = detectar_origen_mensaje(mensaje->origen_mensaje);	//log obligatorio
			log_info(logger, "PID: %d - Acción: ESCRIBIR - Dirección física: %d - Tamaño: %d - Origen: %s", pid, dir_fisica, tamanio_buffer, origen_mensaje_string); //log obligatorio
			free(origen_mensaje_string); //log obligatorio
			
			//-----SALIDA-----//
			usleep(atoi(lectura_de_config.RETARDO_MEMORIA) * 1000); //RETARDO

			switch (mensaje->origen_mensaje){
				case CPU:
					enviar_msj(socket_cpu, ESCRITO_OK);
					break;
				case FILESYSTEM:
					enviar_msj(socket_fileSystem, ESCRITO_OK);
					break;
				default:
					log_error(logger_no_obligatorio, "Origen de mensaje no válido");
					break;
			}
			//----------------//
			
			free(buffer);
			break;
		}
		case LEER_VALOR:{
			//-----ENTRADA-----//
			dir_fisica = atoi(mensaje->parametros[0]);
			tamanio_buffer = atoi(mensaje->parametros[1]); //tamanio del buffer a leer sin /0
			//-----------------//

			buffer = malloc(tamanio_buffer + 1);

			buscar_pid_y_id_segmento_por_dir_fisica(dir_fisica, &pid, &id_segmento);

			hay_seg_fault(pid, id_segmento, dir_fisica, tamanio_buffer);

			memcpy(buffer, memoria_principal + dir_fisica, tamanio_buffer);

			memcpy(buffer + tamanio_buffer, "\0", 1); //agrega el /0 al final del buffer

			char* origen_mensaje_string = detectar_origen_mensaje(mensaje->origen_mensaje);	//log obligatorio
			log_info(logger, "PID: %d - Acción: LEER - Dirección física: %d - Tamaño: %d - Origen: %s", pid, dir_fisica, tamanio_buffer, origen_mensaje_string); //log obligatorio
			free(origen_mensaje_string); //log obligatorio

			//-----SALIDA-----//
			usleep(atoi(lectura_de_config.RETARDO_MEMORIA) * 1000); //RETARDO

			char ** parametros_a_enviar = string_array_new();
			string_array_push(&parametros_a_enviar, buffer);		
			switch (mensaje->origen_mensaje){
				case CPU:
					enviar_msj_con_parametros(socket_cpu, LEIDO_OK, parametros_a_enviar);
					break;
				case FILESYSTEM:
					enviar_msj_con_parametros(socket_fileSystem, LEIDO_OK, parametros_a_enviar);
					break;
				default:
					log_error(logger_no_obligatorio, "Origen de mensaje no válido");
					break;
			}
			string_array_destroy(parametros_a_enviar);
			//----------------//

			break;	//No es necesario el free(buffer) porque se lo libera en el string_array_destroy
		}
		case COMPACTAR:{
			log_info(logger, "Solicitud de Compactación"); //log obligatorio

			compactar();

			log_compactacion(); //log obligatorio			
			
			//-----SALIDA-----//
			usleep(atoi(lectura_de_config.RETARDO_COMPACTACION) * 1000); //RETARDO

			enviar_procesos_con_segmentos_memoria(socket_kernel, lista_procesos);
			//----------------//

			break;
		}
		default:
			string_array_destroy(mensaje->parametros);
			free(mensaje);
			return 0;
		
	}
	string_array_destroy(mensaje->parametros);
	free(mensaje);
	return 1;
}


void compactar() { //CHEQUEADA 2.0
	int pid;
	int id_segmento;

	for(int i = 0; i < bitarray_get_max_bit(bitarray_de_bitmap); i++) {
		if(bitarray_test_bit(bitarray_de_bitmap, i) == 0) {
			for(int j = i; j < bitarray_get_max_bit(bitarray_de_bitmap); j++) {
				if(bitarray_test_bit(bitarray_de_bitmap, j) == 1) {
					buscar_pid_y_id_segmento_por_base(j, &pid, &id_segmento);

					nodoProceso* nodoP = buscar_por_pid(pid);
					nodoSegmento* nodoS = buscar_por_id(nodoP->lista_segmentos, id_segmento);

					void* buffer = malloc(nodoS->tamanio);
					memcpy(buffer, memoria_principal + j, nodoS->tamanio);
					memcpy(memoria_principal + i, buffer, nodoS->tamanio);

					nodoS->base = i;

					for(int h = j; h < j + nodoS->tamanio; h++){
						bitarray_clean_bit(bitarray_de_bitmap, h);
					}

					for(int k = i; k < i + nodoS->tamanio; k++){
						bitarray_set_bit(bitarray_de_bitmap, k);
					}

					free(buffer);
					break;
				}
			}
		}
	}
}

char* detectar_origen_mensaje(int origen_mensaje) {
	char* origen_mensaje_string;
	
	switch (origen_mensaje)	{
		case KERNEL:
			origen_mensaje_string = strdup("KERNEL");
			return origen_mensaje_string;

			break;
		case CPU:
			origen_mensaje_string = strdup("CPU");
			return origen_mensaje_string;

			break;		
		case FILESYSTEM:
			origen_mensaje_string = strdup("FILESYSTEM");
			return origen_mensaje_string;

			break;
		default:
			origen_mensaje_string = strdup("ORIGEN INVALIDO");
			log_error(logger_no_obligatorio, "Origen de mensaje no valido");
			return origen_mensaje_string;

			break;
	}
}


void eliminar_segmento(int pid, int id_segmento) { 
	nodoProceso* nodoP = buscar_por_pid(pid);
	nodoSegmento* nodoS = buscar_por_id(nodoP->lista_segmentos, id_segmento);

	if(nodoS->id != 0){
		for(int i = nodoS->base; i < nodoS->base + nodoS->tamanio; i++){
			bitarray_clean_bit(bitarray_de_bitmap, i);
		}

		memset(memoria_principal + nodoS->base, 0, nodoS->tamanio);
	}

	list_remove_element_memoria(nodoP->lista_segmentos, nodoS); 
	free(nodoS);
}

void eliminar_proceso(int pid) {
	nodoProceso* nodoP = buscar_por_pid(pid);
	nodoSegmento* nodoS;

	for(int i = 0; i < list_size(nodoP->lista_segmentos); i++){
		nodoS = list_get(nodoP->lista_segmentos, i);
		eliminar_segmento(pid, nodoS->id);
	}

	list_destroy(nodoP->lista_segmentos);

	list_remove_element_memoria(lista_procesos, nodoP); 
	free(nodoP);
}

int tengo_espacio_contiguo(int tamanio_segmento) { //CHEQUEADA 2.0
	int contador = 0;

	for(int i = 0; i < bitarray_get_max_bit(bitarray_de_bitmap); i++) {

		if(bitarray_test_bit(bitarray_de_bitmap, i) == 0) {
			contador++;

		} else {
			contador = 0;

		}
		if(contador >= tamanio_segmento) {
			return 1;

		}
	}

	return 0;
}

int tengo_espacio_general(int tamanio_segmento) { //CHEQUEADA 2.0
	int contador = 0;

	for(int i = 0; i < bitarray_get_max_bit(bitarray_de_bitmap); i++) {

		if(bitarray_test_bit(bitarray_de_bitmap, i) == 0) {
			contador++;
		}
	}

	if(contador >= tamanio_segmento) {
		return 1;
	}

	return 0;
}

nodoProceso* crear_proceso(int pid) { //CHEQUEADA 2.0
	nodoProceso* nodoP = malloc(sizeof(nodoProceso));
	nodoP->pid = pid;
	nodoP->lista_segmentos = list_create();

	nodoSegmento* nodoS = malloc(sizeof(nodoSegmento)); //segmento 0
	nodoS->id = 0;
	nodoS->base = 0;
	nodoS->tamanio = atoi(lectura_de_config.TAM_SEGMENTO_0);

	list_add(nodoP->lista_segmentos, nodoS);
	list_add(lista_procesos, nodoP);

	return nodoP;
}

int crear_segmento(int pid, int id_segmento, int tamanio_segmento) { //CHEQUEADA 2.0
	nodoProceso* nodoP = buscar_por_pid(pid);
	int base = asignar_espacio_en_memoria(tamanio_segmento);

	if (list_size(nodoP->lista_segmentos) >= atoi(lectura_de_config.CANT_SEGMENTOS)) {
		log_error(logger_no_obligatorio, "Max Cant de Segmentos alcanzada");
	}
	if (base == -1) {
		log_error(logger_no_obligatorio, "No se pudo asignar un espacio de memoria");
	}

	for(int i = 0; i < tamanio_segmento; i++){
		bitarray_set_bit(bitarray_de_bitmap, base + i);
	}

	nodoSegmento* nodoS = malloc(sizeof(nodoSegmento));
	nodoS->id = id_segmento;
	nodoS->base = base;
	nodoS->tamanio = tamanio_segmento;

	list_add(nodoP->lista_segmentos, nodoS);

	return base;
}

int asignar_espacio_en_memoria(int tamanio_segmento) { //CHEQUEADA 2.0
	int contador = 0;
	int base = -1;
	int espacio_libre = 0;
	
	if(!strcmp("FIRST", lectura_de_config.ALGORITMO_ASIGNACION)) {
		for(int i = 0; i < bitarray_get_max_bit(bitarray_de_bitmap); i++) {
			if(bitarray_test_bit(bitarray_de_bitmap, i) == 0) {
				contador++;
			}
			else {
				contador = 0;
			}
			if(contador == tamanio_segmento) {
				base = i - contador + 1;
				break;
			}
		}
		return base;
	}
		
	else if(!strcmp("BEST", lectura_de_config.ALGORITMO_ASIGNACION)) {
		for(int i = 0; i < bitarray_get_max_bit(bitarray_de_bitmap); i++) {
			if(bitarray_test_bit(bitarray_de_bitmap, i) == 0) {
				contador++;
			}
			else {
				if(contador >= tamanio_segmento && (espacio_libre == 0 || contador < espacio_libre)) {
					espacio_libre = contador;
					base = i - contador;
				}
				contador = 0;
			}
			if(i == bitarray_get_max_bit(bitarray_de_bitmap) - 1){		//caso final de archivo, por si el ultimo es 0
				if(contador >= tamanio_segmento && (espacio_libre == 0 || contador < espacio_libre)) {
					espacio_libre = contador;
					base = i - contador + 1;
				}
			}
		}
		return base;
	}
		
	else if(!strcmp("WORST", lectura_de_config.ALGORITMO_ASIGNACION)) {
		for(int i = 0; i < bitarray_get_max_bit(bitarray_de_bitmap); i++) {
			if(bitarray_test_bit(bitarray_de_bitmap, i) == 0) {
				contador++;
			}
			else {
				if(contador >= tamanio_segmento && contador > espacio_libre) {
					espacio_libre = contador;
					base = i - contador;
				}
				contador = 0;
			}
			if(i == bitarray_get_max_bit(bitarray_de_bitmap) - 1){		//caso final de archivo, por si el ultimo es 0
				if(contador >= tamanio_segmento && contador > espacio_libre) {
					espacio_libre = contador;
					base = i - contador + 1;
				}
			}
		}
		return base;
	}

	log_error(logger_no_obligatorio, "Algoritmo de asignacion no valido"); 
	return base;
}

void hay_seg_fault(int pid, int id_segmento, int dir_fisica, int tamanio_buffer) { //CHEQUEADA 2.0
	nodoProceso* nodoP = buscar_por_pid(pid);
	nodoSegmento* nodoS = buscar_por_id(nodoP->lista_segmentos, id_segmento);

	if(dir_fisica < nodoS->base)
		log_error(logger_no_obligatorio, "Segmentation Fault: Se intento acceder a una direccion de memoria menor a la base del segmento");

	if(dir_fisica + tamanio_buffer > nodoS->base + nodoS->tamanio)
		log_error(logger_no_obligatorio, "Segmentation Fault: Se intento acceder a una direccion de memoria mayor al limite del segmento");

}

//ALGORITMOS LISTAS

nodoProceso* buscar_por_pid(int pid) { //CHEQUEADA 2.0
	
	nodoProceso* nodoP;

	for(int i = 0; i < list_size(lista_procesos); i++) {
		nodoP = list_get(lista_procesos, i);
		if(nodoP->pid == pid) {
			return nodoP;
		}
	}

	log_warning(logger_no_obligatorio, "No se encontro el proceso con pid %d", pid);
	return NULL; 
}

nodoSegmento* buscar_por_id(t_list* lista_segmentos, int id_segmento) { //CHEQUEADA 2.0

	nodoSegmento* nodoS;

	for(int i = 0; i < list_size(lista_segmentos); i++) {
		nodoS = list_get(lista_segmentos, i);
		if(nodoS->id == id_segmento) {
			return nodoS;
		}
	}

	log_warning(logger_no_obligatorio, "No se encontro el segmento con id %d", id_segmento);
	return NULL; 
}

void buscar_pid_y_id_segmento_por_base(int base, int* pid, int* id_segmento) { //CHEQUEADA 2.0
	nodoProceso* nodoP;
	nodoSegmento* nodoS;

	for(int i = 0; i < list_size(lista_procesos); i++) {
		nodoP = list_get(lista_procesos, i);
		for(int j = 0; j < list_size(nodoP->lista_segmentos); j++) {
			nodoS = list_get(nodoP->lista_segmentos, j);
			if(nodoS->base == base) {
				*pid = nodoP->pid;
				*id_segmento = nodoS->id;
				return;
			}
		}
	}

	log_warning(logger_no_obligatorio, "No se encontro el segmento con base %d", base);
	*pid = -1;
	*id_segmento = -1;
	return;
}

void buscar_pid_y_id_segmento_por_dir_fisica(int dir_fisica, int* pid, int* id_segmento) { //CHEQUEADA 2.0
	nodoProceso* nodoP;
	nodoSegmento* nodoS;

	for(int i = 0; i < list_size(lista_procesos); i++) {
		nodoP = list_get(lista_procesos, i);
		for(int j = 0; j < list_size(nodoP->lista_segmentos); j++) {
			nodoS = list_get(nodoP->lista_segmentos, j);
			if(nodoS->base <= dir_fisica && dir_fisica <= (nodoS->base + nodoS->tamanio)) {
				*pid = nodoP->pid;
				*id_segmento = nodoS->id;
				return;
			}
		}
	}

	log_warning(logger_no_obligatorio, "No se encontro el segmento con direccion fisica %d", dir_fisica);
	*pid = -1;
	*id_segmento = -1;
	return;
}

void eliminar_lista_procesos() { 
	nodoProceso* nodoP;

	for(int i = 0; i < list_size(lista_procesos); i++) {
		nodoP = list_get(lista_procesos, i);
		eliminar_proceso(nodoP->pid);
	}

	list_destroy(lista_procesos);

	return;
}

void eliminar_lista_mensajes() {
	t_mensajes* nodoM;

	for(int i = 0; i < list_size(lista_fifo_msj); i++) {
		nodoM = list_get(lista_fifo_msj, i);

		string_array_destroy(nodoM->parametros);

		list_remove_element_memoria(lista_fifo_msj, nodoM); 
		free(nodoM);
	}

	list_destroy(lista_fifo_msj);
}

void log_compactacion() { //CHEQUEADA 2.0
	nodoProceso* nodoP;
	nodoSegmento* nodoS;

	for(int i = 0; i < list_size(lista_procesos); i++) {
		nodoP = list_get(lista_procesos, i);

		for(int j = 0; j < list_size(nodoP->lista_segmentos); j++) {
			nodoS = list_get(nodoP->lista_segmentos, j);
			log_info(logger, "PID: %d - Segmento: %d - Base: %d - Tamaño %d", nodoP->pid, nodoS->id, nodoS->base, nodoS->tamanio); //log obligatorio
		}
	}
}

//SERIALIZACION (JOACO)

void enviar_tabla_segmentos_memoria(int socket, t_list* tabla_segmentos, t_msj_memoria mensaje) {
	size_t size_total;

	void* stream = serializar_tabla_segmentos_memoria(tabla_segmentos, mensaje, &size_total);

	send(socket, stream, size_total, 0);

	free(stream);
}

void* serializar_tabla_segmentos_memoria(t_list* tabla_segmentos, t_msj_memoria mensaje, size_t* size_total) {
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

	nodoSegmento* segmento;

	for(int i = 0; i < list_size(tabla_segmentos); i++) {
		segmento = list_get(tabla_segmentos, i);

		memcpy(stream + desplazamiento, &(segmento->id), sizeof(segmento->id));
		desplazamiento += sizeof(segmento->id);

		memcpy(stream + desplazamiento, &(segmento->base), sizeof(segmento->base));
		desplazamiento += sizeof(segmento->base);

		memcpy(stream + desplazamiento, &(segmento->tamanio), sizeof(segmento->tamanio));
		desplazamiento += sizeof(segmento->tamanio);
	}

	return stream;
}

void enviar_procesos_con_segmentos_memoria(int socket, t_list* procesos_actualizados) {
	size_t size_total;
	void* stream = serializar_procesos_con_segmentos_memoria(procesos_actualizados, &size_total);

	send(socket, stream, size_total, 0);

	free(stream);
}

void* serializar_procesos_con_segmentos_memoria(t_list* procesos_actualizados, size_t* size_total) {
	size_t size_payload = 0;
	*size_total = sizeof(int) + sizeof(size_payload);

	nodoProceso* proceso;
	for(int i = 0; i < list_size(procesos_actualizados); i++) {
		size_payload += sizeof(int) * 2; //pid + cantidad_segmentos

		proceso = list_get(procesos_actualizados, i);
		size_payload += list_size(proceso->lista_segmentos) * sizeof(int) * 3; //id + direccion_base + tamanio //mati: tabla_segmentos
	}

	*size_total += size_payload;

	void* stream = malloc(*size_total);
	int desplazamiento = 0;

	t_msj_memoria op_code = MEMORIA_COMPACTADA;
	memcpy(stream + desplazamiento, &(op_code), sizeof(op_code));
	desplazamiento += sizeof(op_code);

	memcpy(stream + desplazamiento, &size_payload, sizeof(size_payload));
	desplazamiento += sizeof(size_payload);

	nodoSegmento* segmento;
	int cantidad_segmentos;
	for(int i = 0; i < list_size(procesos_actualizados); i++) {
		proceso = list_get(procesos_actualizados, i);
		memcpy(stream + desplazamiento, &(proceso->pid), sizeof(proceso->pid));
		desplazamiento += sizeof(proceso->pid);

		cantidad_segmentos = list_size(proceso->lista_segmentos); //mati: tabla_segmentos
		memcpy(stream + desplazamiento, &cantidad_segmentos, sizeof(cantidad_segmentos));
		desplazamiento += sizeof(cantidad_segmentos);

		for(int j = 0; j < cantidad_segmentos; j++) {
			segmento = list_get(proceso->lista_segmentos, i); //mati: tabla_segmentos

			memcpy(stream + desplazamiento, &(segmento->id), sizeof(segmento->id));
			desplazamiento += sizeof(segmento->id);

			memcpy(stream + desplazamiento, &(segmento->base), sizeof(segmento->base)); //mati: direccion_base
			desplazamiento += sizeof(segmento->base); //mati: direccion_base

			memcpy(stream + desplazamiento, &(segmento->tamanio), sizeof(segmento->tamanio));
			desplazamiento += sizeof(segmento->tamanio);
		}
	}

	return stream;
}

//COSITAS DE COMMONS QUE NO DEBERIAN ESTAR (UPS)

int list_remove_element_memoria(t_list *self, void *element) {
	int _is_the_element(void *data) {
		return element == data;
	}
	return list_remove_by_condition(self, (void* )_is_the_element) != NULL;
}
