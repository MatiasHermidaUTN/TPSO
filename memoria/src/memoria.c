#include "../include/memoria.h"

nodoProceso* lista_procesos = NULL;

void* memoria_principal = malloc(atoi(lectura_de_config.TAM_MEMORIA)); //puntero a memoria principal

void* bitmap_pointer;
t_bitarray* bitarray_de_bitmap;

t_memoria_config lectura_de_config;

int socket_memoria; 

pthread_mutex_t mutex_cola_msj;
sem_t sem_cant_msj;
t_list* lista_fifo_msj;

int main(int argc, char** argv) {
	memset(memoria_principal, 0, atoi(lectura_de_config.TAM_MEMORIA)); //seteo memoria en 0

	t_config* config = iniciar_config(argv[1]);
	lectura_de_config = leer_memoria_config(config);
    t_log* logger = iniciar_logger("memoria.log", "Memoria");

	//INICIO SEMAFORO FIFO de MENSAJES DE KERNEL, CPU y FS
	sem_init(&sem_cant_msj, 0, 0); 		//msj == tarea a ejecutar(escribir, leer, etc.)
	pthread_mutex_init(&mutex_cola_msj, NULL);

	//CREO SERVIDOR Y ESPERO CLIENTES
    socket_memoria = iniciar_servidor("127.0.0.1", lectura_de_config.PUERTO_ESCUCHA); 
	log_warning(logger, "Memoria lista para recibir al cliente");
	while(recibir_conexiones(socket_memoria, logger)); //Recibe conexiones y crea hilos para manejarlas	

	//BITMAP
	int tamanio_bitmap = (int)(atoi(lectura_de_config.TAM_MEMORIA) / 8);
	bitmap_pointer = malloc(tamanio_bitmap); 
	if(!(atoi(lectura_de_config.TAM_MEMORIA) % 8)){ //error por si el bitmap no tiene el tamaño correcto
		//error
	}
	bitarray_de_bitmap = bitarray_create_with_mode(bitmap_pointer, tamanio_bitmap, MSB_FIRST);

	log_info(logger, "Bitmap creado");

	//Segmento 0
	for(int i = 0; i < atoi(lectura_de_config.TAM_SEGMENTO_0); i++){
		bitarray_set_bit(bitarray_de_bitmap, i);
	}
	log_info(logger, "segmento 0 creado en el bitmap");

	while(manejar_mensaje());

	log_destroy(logger);
	config_destroy(config);
	free(memoria_principal);
	eliminar_lista_proceso(&lista_procesos);

	return EXIT_SUCCESS;
}

int manejar_mensaje() { //pinta bien
	
	sem_wait(&sem_cant_msj);

	t_mensajes* mensaje = list_pop_con_mutex(lista_fifo_msj, &mutex_cola_msj);

	t_instrucciones cod_op = ((t_mensajes*)mensaje)->cod_op;
	int pid = ((t_mensajes*)mensaje)->pid;
	int id_segmento = ((t_mensajes*)mensaje)->id_segmento;
	int tamanio_segmento = ((t_mensajes*)mensaje)->tamanio_segmento;
	int dir_fisica = ((t_mensajes*)mensaje)->dir_fisica;
	int tamanio_buffer = ((t_mensajes*)mensaje)->tamanio_buffer;
	char* buffer = strdup(((t_mensajes*)mensaje)->buffer);
	t_origen_mensaje origen_mensaje = ((t_mensajes*)mensaje)->origen_mensaje;

	char* bufferLectura = malloc(tamanio_buffer);

	switch (cod_op) {
		case INICIALIZAR_EL_PROCESO:{
			//parametros a recibir (necesarios) en este caso: pid, origen_mensaje 	

			log_info(logger, "Creación de Proceso PID: <%d>", pid); //log obligatorio

			nodoProceso* nodoP = crear_proceso(pid);			

			//msj kernel proceso creado y tabla actualizada

			free(buffer);
			free(bufferLectura);
			free(origen_mensaje);
			break;
		}
		case CREAR_SEGMENTO:{
			//parametros a recibir (necesarios) en este caso: pid, tamanio_segmento, origen_mensaje
			if(!tengo_espacio_general(tamanio_segmento)) {
				//error?
			}
			if(!tengo_espacio_contiguo(tamanio_segmento)) {
				//msj kernel compactacion
			}
			int base = crear_segmento(pid, tamanio_segmento); //TODO definir id_segmento yo

			log_info(logger, "PID: <%d> - Crear Segmento: <%d> - Base: <%d> - TAMAÑO: <%d>", pid, id_segmento, base, tamanio_segmento); //log obligatorio

			//msj kernel segmento creado y base

			free(buffer);
			free(bufferLectura);
			free(origen_mensaje);
			break;
		}
		case ELIMINAR_SEGMENTO:{
			//parametros a recibir (necesarios) en este caso: pid, id_segmento, origen_mensaje

			nodoProceso* nodoP = buscar_por_pid(lista_procesos, pid);
			nodoSegmento* nodoS = buscar_por_id(nodoP->lista_segmentos, id_segmento);

			int base = nodoS->base;
			int tamanio_segmento = nodoS->tamanio;
			
			eliminar_segmento(pid, id_segmento);

			nodoProceso* nodoP = buscar_por_pid(lista_procesos, pid);

			log_info(logger, "PID: <%d> - Eliminar Segmento: <%d> - Base: <%d> - TAMAÑO: <%d>", pid, id_segmento, base, tamanio_segmento); //log obligatorio

			//msj kernel segmento eliminado y tabla actualizada (nodop->lista_segmentos)

			free(buffer);
			free(bufferLectura);
			free(origen_mensaje);
			break;
		}
		case ELIMINAR_PROCESO:{
			//parametros a recibir (necesarios) en este caso: pid, id_segmento, origen_mensaje

			eliminar_proceso(pid);

			log_info(logger, "Eliminación de Proceso PID: <%d>", pid); //log obligatorio

			//msj kernel proceso eliminado 

			free(buffer);
			free(bufferLectura);
			free(origen_mensaje);
			break;
		}
		case ESCRIBIR:{
			//parametros a recibir (necesarios) en este caso: pid, id_segmento, dir_fisica, buffer, tamanio_buffer, origen_mensaje
			if(hay_seg_fault(pid, id_segmento, dir_fisica, tamanio_buffer)){
				//msj kernel seg fault
			}

			memcpy(memoria_principal + dir_fisica, buffer, tamanio_buffer);

			log_info(logger, "PID: <%d> - Acción: <ESCRIBIR> - Dirección física: <%d> - Tamaño: <%d> - Origen: <%s>", pid, dir_fisica, tamanio_buffer, origen_mensaje); //log obligatorio

			//msj kernel escrito

			free(buffer);
			free(bufferLectura);
			free(origen_mensaje);
			break;
		}
		case LEER:{
			//parametros a recibir (necesarios) en este caso: pid, id_segmento, dir_fisica, buffer, tamanio_buffer, origen_mensaje
			if(hay_seg_fault(pid, id_segmento, dir_fisica, tamanio_buffer)){
				//msj kernel seg fault
			}

			memcpy(bufferLectura, memoria_principal + dir_fisica, tamanio_buffer);

			log_info(logger, "PID: <%d> - Acción: <LEER> - Dirección física: <%d> - Tamaño: <%d> - Origen: <%s>", pid, dir_fisica, tamanio_buffer, origen_mensaje); //log obligatorio

			//enviar bufferLectura

			free(buffer);
			free(bufferLectura);
			free(origen_mensaje);
			break;
		}
		case COMPACTAR:{
			//parametros a recibir (necesarios) en este caso: origen_mensaje
			log_info(logger, "Solicitud de Compactación"); //log obligatorio

			compactar();

			log_compactacion(); //log obligatorio			

			//msj kernel compactacion done y devuelvo lista_procesos

			free(buffer);
			free(bufferLectura);
			free(origen_mensaje);

			break;
		}
		case ERROR:{
			//msj kernel error

			free(buffer);
			free(bufferLectura);
			free(origen_mensaje);

			break;
		}
		default:{ 
			free(buffer);
			free(bufferLectura);
			free(origen_mensaje);
			return 0;
		}
	}
	return 1;
}


void compactar() { //CHEQUEADA
	int pid;
	int id_segmento;
	for(int i = 0; i < bitarray_get_max_bit(bitarray_de_bitmap); i++) {
		if(bitarray_test_bit(bitarray_de_bitmap, i) == 0) {
			for(int j = i; j < bitarray_get_max_bit(bitarray_de_bitmap); j++) {
				if(bitarray_test_bit(bitarray_de_bitmap, j) == 1) {
					buscar_por_base(lista_procesos, j, &pid, &id_segmento);

					nodoProceso* nodoP = buscar_por_pid(lista_procesos, pid);
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

int hay_seg_fault(int pid, int id_segmento, int dir_fisica, int tamanio_buffer) { //CHEQUEADA
	nodoProceso* nodoP = buscar_por_pid(lista_procesos, pid);
	nodoSegmento* nodoS = buscar_por_id(nodoP->lista_segmentos, id_segmento);

	if(dir_fisica < nodoS->base) 
		return 1;	

	if(dir_fisica + tamanio_buffer > nodoS->base + nodoS->tamanio) 
		return 1;
	
	return 0;
}

void eliminar_segmento(int pid, int id_segmento) { //CHEQUEADA
	nodoProceso* nodoP = buscar_por_pid(lista_procesos, pid);
	nodoSegmento* nodoS = buscar_por_id(nodoP->lista_segmentos, id_segmento);

	for(int i = nodoS->base; i < nodoS->base + nodoS->tamanio; i++){
		bitarray_clean_bit(bitarray_de_bitmap, i);
	}

	borrar_nodo_segmento(&(nodoP->lista_segmentos), nodoS);	
}

void eliminar_proceso(int pid) {
	nodoProceso* nodoP = buscar_por_pid(lista_procesos, pid);

	while(nodoP->lista_segmentos != NULL){
		eliminar_segmento(pid, nodoP->lista_segmentos->id); //borro siempre el primer nodo, funca pero no se si es lo mejor
	}

	borrar_nodo_proceso(&lista_procesos, nodoP);
}

int tengo_espacio_contiguo(int tamanio_segmento){ //CHEQUEADA
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

int tengo_espacio_general(int tamanio_segmento) { //CHEQUEADA
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

nodoProceso* crear_proceso(int pid){ //CHEQUEADA
	nodoProceso* nodoP = malloc(sizeof(nodoProceso));
	nodoP->pid = pid;

	nodoSegmento* nodoS = malloc(sizeof(nodoSegmento)); //segmento 0
	nodoS->id = 0;
	nodoS->base = 0;
	nodoS->tamanio = atoi(lectura_de_config.TAM_SEGMENTO_0);
	nodoS->siguiente = NULL;

	nodoP->lista_segmentos = nodoS;
	nodoP->siguiente = NULL;

	push_proceso(&lista_procesos, nodoP);

	nodoProceso* nodoPaux = buscar_por_pid(lista_procesos, pid);

	return nodoPaux;
}

void asignar_id_segmento(int pid){
	nodoProceso* nodoP = buscar_por_pid(lista_procesos, pid);

	int i = -1;

	for(; (i < atoi(lectura_de_config.CANT_SEGMENTOS)) && (buscar_por_id(nodoP->lista_segmentos, i) != NULL); i++)
	{}

	if(i == -1){
		//error
	}

	return i;
}

int crear_segmento(int pid, int tamanio_segmento) { //CHEQUEADA
	nodoProceso* nodoP = buscar_por_pid(lista_procesos, pid);
	int base = asignar_espacio_en_memoria(tamanio_segmento);

	if (length_segmento(nodoP->lista_segmentos) >= atoi(lectura_de_config.CANT_SEGMENTOS)) {
		//error
	}
	if (base == -1) {
		//error
	}

	for(int i = base; i < base + tamanio_segmento; i++){
		bitarray_set_bit(bitarray_de_bitmap, i);
	}

	nodoSegmento* nodoS = malloc(sizeof(nodoSegmento));
	nodoS->id = asignar_id_segmento(pid);
	nodoS->base = base;
	nodoS->tamanio = tamanio_segmento;
	nodoS->siguiente = NULL;

	push_segmento(&(nodoP->lista_segmentos), nodoS);

	return base;
}

int asignar_espacio_en_memoria(int tamanio_segmento) { //CHEQUEADA
	int contador = 0;
	int base = -1;
	int espacio_libre = 0;
	t_algoritmo_asignacion algoritmo_asignacion;
	
	if(!strcmp("FIRST", lectura_de_config.ALGORITMO_ASIGNACION))
		algoritmo_asignacion = FIRST;
	else if(!strcmp("BEST", lectura_de_config.ALGORITMO_ASIGNACION))
		algoritmo_asignacion = BEST;
	else if(!strcmp("WORST", lectura_de_config.ALGORITMO_ASIGNACION))
		algoritmo_asignacion = WORST;

	switch(algoritmo_asignacion){
		default:
		case FIRST:{
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
		case BEST:{
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
		case WORST:{
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
	}
}

//ALGORITMOS LISTAS

nodoProceso* buscar_por_pid(nodoProceso* lista_procesos, int pid) { //CHEQUEADA
	nodoProceso* nodoP = lista_procesos;

	while(nodoP->pid != pid){
		nodoP = nodoP->siguiente;
	}

	if(nodoP->pid != pid){
		//error?
		return NULL; //puede llegar a romper cuando se intente usar (eso seria bueno)
	}

	return nodoP;
}

nodoSegmento* buscar_por_id(nodoSegmento* lista_segmentos, int id_segmento) { //CHEQUEADA
	nodoSegmento* nodoS = lista_segmentos;

	while(nodoS->id != id_segmento){
		nodoS = nodoS->siguiente;
	}

	if(nodoS->id != id_segmento){
		//error?
		return NULL; //puede llegar a romper cuando se intente usar (eso seria bueno)
	}

	return nodoS;
}

void borrar_nodo_segmento(nodoSegmento** referenciaListaSeg, nodoSegmento* nodo_a_borrar) { //pinta bien
	nodoSegmento* nodoS = *referenciaListaSeg;
	nodoSegmento* nodoAnterior = NULL;

	while (nodoS && (nodoS != nodo_a_borrar))
	{
		nodoAnterior = nodoS;
		nodoS = nodoS->siguiente;
	}

	if (nodoS)
	{
		if (nodoAnterior)  // check principio de la lista
			nodoAnterior->siguiente = nodoS->siguiente;
		else
			*referenciaListaSeg = nodoS->siguiente;

		free(nodoS);
	}
	return;

}

void borrar_nodo_proceso(nodoProceso** lista_procesos, nodoProceso* nodo_a_borrar){
	nodoProceso* nodoP = *lista_procesos;
	nodoProceso* nodoAnterior = NULL;

	while (nodoP && (nodoP != nodo_a_borrar)) {
		nodoAnterior = nodoP;
		nodoP = nodoP->siguiente;
	}

	if(nodoP){
		if(nodoAnterior){  // check principio de la lista
			nodoAnterior->siguiente = nodoP->siguiente;
		}
		else{
			*lista_procesos = nodoP->siguiente;
		}
		free(nodoP);
	}
	return;

}

int length_segmento(nodoSegmento* nodoS) { //CHEQUEADA
	int contador = 0;

	while(nodoS != NULL){
		contador++;
		nodoS = nodoS->siguiente;
	}

	return contador;
}

void buscar_por_base(nodoProceso* nodoP, int base, int* pid, int* id_segmento) { //CHEQUEADA
	while(nodoP != NULL){
		nodoSegmento* nodoS = nodoP->lista_segmentos;
		while(nodoS != NULL){
			if(nodoS->base == base){
				*pid = nodoP->pid;
				*id_segmento = nodoS->id;
				return;
			}
			nodoS = nodoS->siguiente;
		}
		nodoP = nodoP->siguiente;
	}
}

void push_segmento(nodoSegmento** referencia_lista, nodoSegmento* nodoS) { //pinta bien
    nodoSegmento* paux;
    if (*referencia_lista)
    {
        while (paux && paux->siguiente) //recorre hasta el ultimo nodo
        	paux = paux->siguiente;

        paux->siguiente = malloc(sizeof(nodoSegmento));
        paux->siguiente = nodoS;
    }
    else //crea primer nodo
    {
        *referencia_lista = malloc(sizeof(nodoSegmento));
        *referencia_lista = nodoS;
    }
    return;
}

void push_proceso(nodoProceso** referencia_lista, nodoProceso* nodoP) { //pinta bien
    nodoProceso* paux;
    if (*referencia_lista)
    {
        while (paux && paux->siguiente) //recorre hasta el ultimo nodo
        	paux = paux->siguiente;

        paux->siguiente = malloc(sizeof(nodoProceso));
        paux->siguiente = nodoP;
    }
    else //crea primer nodo
    {
        *referencia_lista = malloc(sizeof(nodoProceso));
        *referencia_lista = nodoP;
    }
    return;
}

void eliminar_lista_proceso(nodoProceso** lista_procesos){ //pinta bien
	nodoProceso* nodoP = *lista_procesos;
	nodoProceso* nodoP2 = NULL;

	nodoSegmento* nodoS = NULL;
	nodoSegmento* nodoS2 = NULL;

	while(nodoP != NULL){
		nodoS = nodoP->lista_segmentos;

		while(nodoS != NULL){
			nodoS2 = nodoS->siguiente;
			free(nodoS);
			nodoS = nodoS2;
		}

		nodoP2 = nodoP->siguiente;
		free(nodoP);
		nodoP = nodoP2;
	}

	*lista_procesos = NULL;
	return;
}

void log_compactacion(){ //CHEQUEADA
	nodoProceso* nodoP = lista_procesos;
	while(nodoP != NULL){
		int pid =  nodoP->pid;
		nodoSegmento* nodoS = nodoP->lista_segmentos;
		while(nodoS != NULL){
			int id_segmento =  nodoS->id;
			int base =  nodoS->base;
			int tamanio_segmento = nodoS->tamanio;

			log_info(logger, "PID: <%d> - Segmento: <%d> - Base: <%d> - Tamaño <%d>", pid, id_segmento, base, tamanio_segmento); //log obligatorio

			nodoS = nodoS->siguiente;
		}
		nodoP = nodoP->siguiente;
	}
}

///// PRUEBAS ////////////////////

void imprimir_bitmap() {
	for(int i = 0; i < 400/*bitarray_get_max_bit(bitarray_de_bitmap)*/; i++) {
		printf("%d ", bitarray_test_bit(bitarray_de_bitmap, i));
		if(i+1 % 100 == 0) {
			printf("\n");
		}
	}
}

void imprimir_lista() {
	nodoProceso* nodoP = lista_procesos;
	while(nodoP != NULL){
		printf("PID: %d\n", nodoP->pid);
		nodoSegmento* nodoS = nodoP->lista_segmentos;
		while(nodoS != NULL){
			printf("ID: %d\n", nodoS->id);
			printf("Base: %d\n", nodoS->base);
			printf("Tamanio: %d\n", nodoS->tamanio);
			nodoS = nodoS->siguiente;
		}
		nodoP = nodoP->siguiente;
		printf("------------------------------------------");
		printf("\n");
	}
}

void imprimir_memoria(int desde, int hasta) {
	for(int i = desde; i < hasta; i++) {
		printf(" %d ", ((int*)memoria_principal)[i]);
		if(i % 100 == 0) {
			printf("\n");
		}
	}
}

/* Ni idea q hace esto, estaba arriba de todo. Por las dudas lo deje aca :)
void sighandler(int x) {
    switch (x) {
        case SIGINT:
        	close(socket_memoria);
            exit(EXIT_SUCCESS);
    }
}*/
