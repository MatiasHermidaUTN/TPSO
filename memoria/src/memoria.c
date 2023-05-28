#include "../include/memoria.h"

nodoProceso* lista_procesos;

void* memoria_principal;

void* bitmap_pointer;
t_bitarray* bitarray_de_bitmap;

t_memoria_config lectura_de_config;

int socket_memoria;

pthread_mutex_t mutex_cola_msj;
sem_t sem_cant_msj;
t_list* lista_fifo_msj;
t_log* logger;

int main(int argc, char** argv) {
	t_config* config = iniciar_config(argv[1]);
	lectura_de_config = leer_memoria_config(config);
    logger = iniciar_logger("memoria.log", "Memoria");

    lista_fifo_msj = list_create();
	lista_procesos = NULL;
	memoria_principal = malloc(atoi(lectura_de_config.TAM_MEMORIA)); //puntero a memoria principal

	memset(memoria_principal, 0, atoi(lectura_de_config.TAM_MEMORIA)); //seteo memoria en 0

	//INICIO SEMAFORO FIFO de MENSAJES DE KERNEL, CPU y FS
	sem_init(&sem_cant_msj, 0, 0); 		//msj == tarea a ejecutar(escribir, leer, etc.)
	pthread_mutex_init(&mutex_cola_msj, NULL);

	//CREO SERVIDOR Y ESPERO CLIENTES
    socket_memoria = iniciar_servidor("127.0.0.1", lectura_de_config.PUERTO_ESCUCHA); 
	log_warning(logger, "Memoria lista para recibir al cliente");
	/*for(int i = 0 ; i<3 ; ){		//3 pq hay que aceptar 1 de cada: Kernel, CPU y FileSystem
		i += recibir_conexiones(socket_memoria, logger); //Recibe conexiones (es bloqueante) y crea hilos para manejarlas
	}*/

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

	//TODO tests
	printf("empezando pruebas:\n");
	prueba();
	//sleep(3);
	//tests

	while(manejar_mensaje());

	log_destroy(logger);
	config_destroy(config);
	free(memoria_principal);
	eliminar_lista_proceso(&lista_procesos);

	return EXIT_SUCCESS;
}

int manejar_mensaje() { //pinta bien
	printf("\nesperando\n");
	sem_wait(&sem_cant_msj);

	void* mensaje = list_pop_con_mutex(lista_fifo_msj, &mutex_cola_msj);
	t_instrucciones cod_op = ((t_mensajes*)mensaje)->cod_op;
	printf("cod_op: %d\n", cod_op);
	int pid = ((t_mensajes*)mensaje)->pid;
	printf("pid: %d\n", pid);
	int id_segmento = ((t_mensajes*)mensaje)->id_segmento;
	printf("id_segmento: %d\n", id_segmento);
	int tamanio_segmento = ((t_mensajes*)mensaje)->tamanio_segmento;
	printf("tamanio_segmento: %d\n", tamanio_segmento);
	int dir_fisica = ((t_mensajes*)mensaje)->dir_fisica;
	printf("dir_fisica: %d\n", dir_fisica);
	int tamanio_buffer = ((t_mensajes*)mensaje)->tamanio_buffer;
	printf("tamanio_buffer: %d\n", tamanio_buffer);
	char* buffer = strdup(((t_mensajes*)mensaje)->buffer);
	printf("buffer: %s\n", buffer);
	t_handshake origen_mensaje = ((t_mensajes*)mensaje)->origen_mensaje;
	printf("origen_mensaje: %d\n", origen_mensaje);

	char* bufferLectura = malloc(tamanio_buffer + 1);		//TODO borrar el +1 cuando hagas serializar envio de lectura

	free(mensaje);
	sleep(1);

	switch (cod_op) {
		case INICIALIZAR_EL_PROCESO:{
			//parametros a recibir (necesarios) en este caso: pid, origen_mensaje 	

			log_info(logger, "Creación de Proceso PID: <%d>", pid); //log obligatorio

			nodoProceso* nodoP = crear_proceso(pid);

			//msj kernel proceso creado y tabla actualizada

			free(buffer);
			free(bufferLectura);
			break;
		}
		case CREAR_SEG:{
			//parametros a recibir (necesarios) en este caso: pid, id_segmento, tamanio_segmento, origen_mensaje
			if(!tengo_espacio_general(tamanio_segmento)) {
				//error?
				log_error(logger, "no hay espacio para segmento");
			}
			if(!tengo_espacio_contiguo(tamanio_segmento)) {
				//msj kernel compactacion
				log_error(logger, "no hay espacio para segmento contiguo");
			}
			int base = crear_segmento(pid, id_segmento, tamanio_segmento); //TODO definir id_segmento yo

			log_info(logger, "PID: <%d> - Crear Segmento: <%d> - Base: <%d> - TAMAÑO: <%d>", pid, id_segmento, base, tamanio_segmento); //log obligatorio

			//msj kernel segmento creado y base

			free(buffer);
			free(bufferLectura);
			break;
		}
		case ELIMINAR_SEG:{
			//parametros a recibir (necesarios) en este caso: pid, id_segmento, origen_mensaje

			nodoProceso* nodoP = buscar_por_pid(lista_procesos, pid);
			nodoSegmento* nodoS = buscar_por_id(nodoP->lista_segmentos, id_segmento);

			int base = nodoS->base;
			int tamanio_segmento = nodoS->tamanio;
			
			eliminar_segmento(pid, id_segmento);

			nodoP = buscar_por_pid(lista_procesos, pid);

			log_info(logger, "PID: <%d> - Eliminar Segmento: <%d> - Base: <%d> - TAMAÑO: <%d>", pid, id_segmento, base, tamanio_segmento); //log obligatorio

			//msj kernel segmento eliminado y tabla actualizada (nodop->lista_segmentos)

			free(buffer);
			free(bufferLectura);
			break;
		}
		case ELIMINAR_PROCESO:{
			//parametros a recibir (necesarios) en este caso: pid, id_segmento, origen_mensaje

			eliminar_proceso(pid);

			log_info(logger, "Eliminación de Proceso PID: <%d>", pid); //log obligatorio

			//msj kernel proceso eliminado 

			free(buffer);
			free(bufferLectura);
			break;
		}
		case ESCRIBIR:{
			//parametros a recibir (necesarios) en este caso: dir_fisica, buffer, tamanio_buffer, origen_mensaje
			int pid;
			buscar_pid_por_dir_fisica(dir_fisica, &pid);

			memcpy(memoria_principal + dir_fisica, buffer, tamanio_buffer);

			log_info(logger, "PID: <%d> - Acción: <ESCRIBIR> - Dirección física: <%d> - Tamaño: <%d> - Origen: <%d>", pid, dir_fisica, tamanio_buffer, origen_mensaje); //log obligatorio

			//msj kernel escrito

			free(buffer);
			free(bufferLectura);
			break;
		}
		case LEER:{
			//parametros a recibir (necesarios) en este caso: dir_fisica, buffer, tamanio_buffer, origen_mensaje
			int pid;
			buscar_pid_por_dir_fisica(dir_fisica, &pid);

			memcpy(bufferLectura, memoria_principal + dir_fisica, tamanio_buffer);

			log_info(logger, "PID: <%d> - Acción: <LEER> - Dirección física: <%d> - Tamaño: <%d> - Origen: <%d>", pid, dir_fisica, tamanio_buffer, origen_mensaje); //log obligatorio

			//enviar bufferLectura, hay que agregar 1 byte más con el \0 en el envío del msj
			memcpy(bufferLectura + tamanio_buffer, "\0", 1);	//TODO esto puede q sea una forrada, ver si sacarlo o hacerlo en otro lado
			printf("bufferLectura: %s\n", bufferLectura);

			free(buffer);
			free(bufferLectura);
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

			break;
		}
		case ERROR:{
			//msj kernel error

			free(buffer);
			free(bufferLectura);

			break;
		}
		default:{ 
			free(buffer);
			free(bufferLectura);
			return 0;
		}
	}
	printf("\n");
	imprimir_bitmap();
	printf("\n");
	printf("\n");
	imprimir_memoria(0, 400);
	printf("\n");
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

void eliminar_segmento(int pid, int id_segmento) { //CHEQUEADA
	nodoProceso* nodoP = buscar_por_pid(lista_procesos, pid);
	nodoSegmento* nodoS = buscar_por_id(nodoP->lista_segmentos, id_segmento);

	if(nodoS->id != 0){
		for(int i = nodoS->base; i < nodoS->base + nodoS->tamanio; i++){
			bitarray_clean_bit(bitarray_de_bitmap, i);
		}

		memset(memoria_principal + nodoS->base, 0, nodoS->tamanio);
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
	return nodoP;
}

int crear_segmento(int pid, int id_segmento, int tamanio_segmento) { //CHEQUEADA
	nodoProceso* nodoP = buscar_por_pid(lista_procesos, pid);
	int base = asignar_espacio_en_memoria(tamanio_segmento);

	if (length_segmento(nodoP->lista_segmentos) >= atoi(lectura_de_config.CANT_SEGMENTOS)) {
		//error
		log_error(logger, "Max Cant de Segmentos alcanzada");
	}
	if (base == -1) {
		//error
		log_error(logger, "No se pudo asignar un espacio de memoria");
	}

	for(int i = 0; i < tamanio_segmento; i++){
		bitarray_set_bit(bitarray_de_bitmap, base + i);
	}

	nodoSegmento* nodoS = malloc(sizeof(nodoSegmento));
	nodoS->id = id_segmento;
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

	while(nodoP != NULL && nodoP->pid != pid){
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

	while(nodoS != NULL && nodoS->id != id_segmento){
		nodoS = nodoS->siguiente;
	}

	if(nodoS == NULL || nodoS->id != id_segmento){
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
		if (nodoAnterior)  // check principio de la lista (!= NULL)
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

void buscar_pid_por_dir_fisica(int dir_fisica, int *pid) {
	nodoProceso* nodoP = lista_procesos;

	while(nodoP != NULL){
		nodoSegmento* nodoS = nodoP->lista_segmentos;

		while(nodoS != NULL){
			if((nodoS->base <= dir_fisica) && (dir_fisica <= (nodoS->base + nodoS->tamanio))){
				*pid = nodoP->pid;
				return;
			}
			nodoS = nodoS->siguiente;
		}
		nodoP = nodoP->siguiente;
	}

	if(nodoP == NULL){
		//error?
	}
}

void push_segmento(nodoSegmento** referencia_lista, nodoSegmento* nodoS) { //pinta bien
    nodoSegmento* paux = *referencia_lista;
    if (*referencia_lista)
    {
        while (paux && paux->siguiente) //recorre hasta el ultimo nodo
        	paux = paux->siguiente;

        paux->siguiente = nodoS;
    }
    else //crea primer nodo
    {
        *referencia_lista = nodoS;
    }
    return;
}

void push_proceso(nodoProceso** referencia_lista, nodoProceso* nodoP) { //pinta bien
    nodoProceso* paux = *referencia_lista;
    if (*referencia_lista)
    {
        while (paux && paux->siguiente) //recorre hasta el ultimo nodo
        	paux = paux->siguiente;

        paux->siguiente = nodoP;
    }
    else //crea primer nodo
    {
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
	printf("bitmap:\n");
	for(int i = 0; i < 400/*bitarray_get_max_bit(bitarray_de_bitmap)*/; i++) {	//TODO cambiar el 400 por lo comentado
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
	printf("memoria:\n");
	for(int i = desde; i < hasta; i++) {
		printf("%d ", ((char*)memoria_principal)[i]);
		if(i+1 % 100 == 0) {
			printf("\n");
		}
	}
}

void prueba(){
	//TODO test
	/*for(int i = atoi(lectura_de_config.TAM_SEGMENTO_0)+1; i < atoi(lectura_de_config.TAM_SEGMENTO_0)+1+3; i++){
		bitarray_set_bit(bitarray_de_bitmap, i);
	}
	for(int i = atoi(lectura_de_config.TAM_SEGMENTO_0)+1+3+5; i < atoi(lectura_de_config.TAM_SEGMENTO_0)+1+3+5+7; i++){
		bitarray_set_bit(bitarray_de_bitmap, i);
	}
	for(int i = atoi(lectura_de_config.TAM_SEGMENTO_0)+1+3+5+7+4; i < atoi(lectura_de_config.TAM_SEGMENTO_0)+1+3+5+7+4+2; i++){
		bitarray_set_bit(bitarray_de_bitmap, i);
	}*/

	printf("\n");
	imprimir_bitmap();
	printf("\n");
	printf("\n");
	imprimir_memoria(0, 400);
	printf("\n");

	t_mensajes* mensaje = malloc(sizeof(t_mensajes));
	mensaje->cod_op = INICIALIZAR_EL_PROCESO;
	mensaje->pid = 2;
	mensaje->id_segmento = 0;
	mensaje->tamanio_segmento = 0;
	mensaje->dir_fisica = 0;
	mensaje->tamanio_buffer = 0;
	mensaje->buffer = strdup("");
	mensaje->origen_mensaje = KERNEL;
	list_push_con_mutex(lista_fifo_msj, mensaje, &mutex_cola_msj);
	sem_post(&sem_cant_msj);

	t_mensajes* mensaje2 = malloc(sizeof(t_mensajes));
	mensaje2->cod_op = CREAR_SEG;
	mensaje2->pid = 2;
	mensaje2->id_segmento = 0;
	mensaje2->tamanio_segmento = 10;
	mensaje2->dir_fisica = 0;
	mensaje2->tamanio_buffer = 0;
	mensaje2->buffer = strdup("\0");
	mensaje2->origen_mensaje = KERNEL;
	list_push_con_mutex(lista_fifo_msj, mensaje2, &mutex_cola_msj);
	sem_post(&sem_cant_msj);

	t_mensajes* mensaje3 = malloc(sizeof(t_mensajes));
	mensaje3->cod_op = CREAR_SEG;
	mensaje3->pid = 2;
	mensaje3->id_segmento = 0;
	mensaje3->tamanio_segmento = 5;
	mensaje3->dir_fisica = 0;
	mensaje3->tamanio_buffer = 0;
	mensaje3->buffer = strdup("\0");
	mensaje3->origen_mensaje = KERNEL;
	list_push_con_mutex(lista_fifo_msj, mensaje3, &mutex_cola_msj);
	sem_post(&sem_cant_msj);

	t_mensajes* mensaje35 = malloc(sizeof(t_mensajes));
	mensaje35->cod_op = CREAR_SEG;
	mensaje35->pid = 2;
	mensaje35->id_segmento = 0;
	mensaje35->tamanio_segmento = 7;
	mensaje35->dir_fisica = 0;
	mensaje35->tamanio_buffer = 0;
	mensaje35->buffer = strdup("\0");
	mensaje35->origen_mensaje = KERNEL;
	list_push_con_mutex(lista_fifo_msj, mensaje35, &mutex_cola_msj);
	sem_post(&sem_cant_msj);

	t_mensajes* mensaje4 = malloc(sizeof(t_mensajes));
	mensaje4->cod_op = ELIMINAR_SEG;
	mensaje4->pid = 2;
	mensaje4->id_segmento = 2;
	mensaje4->tamanio_segmento = 0;
	mensaje4->dir_fisica = 0;
	mensaje4->tamanio_buffer = 0;
	mensaje4->buffer = strdup("\0");
	mensaje4->origen_mensaje = KERNEL;
	list_push_con_mutex(lista_fifo_msj, mensaje4, &mutex_cola_msj);
	sem_post(&sem_cant_msj);

	t_mensajes* mensaje5 = malloc(sizeof(t_mensajes));
	mensaje5->cod_op = ESCRIBIR;
	mensaje5->pid = 2;
	mensaje5->id_segmento = 3;
	mensaje5->tamanio_segmento = 0;
	mensaje5->dir_fisica = 145;
	mensaje5->tamanio_buffer = 3;
	mensaje5->buffer = strdup("ABC");
	mensaje5->origen_mensaje = CPU;
	list_push_con_mutex(lista_fifo_msj, mensaje5, &mutex_cola_msj);
	sem_post(&sem_cant_msj);

	t_mensajes* mensaje10 = malloc(sizeof(t_mensajes));
	mensaje10->cod_op = INICIALIZAR_EL_PROCESO;
	mensaje10->pid = 1;
	mensaje10->id_segmento = 0;
	mensaje10->tamanio_segmento = 0;
	mensaje10->dir_fisica = 0;
	mensaje10->tamanio_buffer = 0;
	mensaje10->buffer = strdup("");
	mensaje10->origen_mensaje = KERNEL;
	list_push_con_mutex(lista_fifo_msj, mensaje10, &mutex_cola_msj);
	sem_post(&sem_cant_msj);

	t_mensajes* mensaje11 = malloc(sizeof(t_mensajes));
	mensaje11->cod_op = CREAR_SEG;
	mensaje11->pid = 1;
	mensaje11->id_segmento = 0;
	mensaje11->tamanio_segmento = 15;
	mensaje11->dir_fisica = 0;
	mensaje11->tamanio_buffer = 0;
	mensaje11->buffer = strdup("\0");
	mensaje11->origen_mensaje = KERNEL;
	list_push_con_mutex(lista_fifo_msj, mensaje11, &mutex_cola_msj);
	sem_post(&sem_cant_msj);

	t_mensajes* mensaje6 = malloc(sizeof(t_mensajes));
	mensaje6->cod_op = COMPACTAR;
	mensaje6->pid = 0;
	mensaje6->id_segmento = 0;
	mensaje6->tamanio_segmento = 0;
	mensaje6->dir_fisica = 0;
	mensaje6->tamanio_buffer = 0;
	mensaje6->buffer = strdup("\0");
	mensaje6->origen_mensaje = KERNEL;
	list_push_con_mutex(lista_fifo_msj, mensaje6, &mutex_cola_msj);
	sem_post(&sem_cant_msj);

	t_mensajes* mensaje7 = malloc(sizeof(t_mensajes));
	mensaje7->cod_op = LEER;
	mensaje7->pid = 2;
	mensaje7->id_segmento = 3;
	mensaje7->tamanio_segmento = 0;
	mensaje7->dir_fisica = 140;
	mensaje7->tamanio_buffer = 3;
	mensaje7->buffer = strdup("\0");
	mensaje7->origen_mensaje = CPU;
	list_push_con_mutex(lista_fifo_msj, mensaje7, &mutex_cola_msj);
	sem_post(&sem_cant_msj);

	t_mensajes* mensaje8 = malloc(sizeof(t_mensajes));
	mensaje8->cod_op = ELIMINAR_PROCESO;
	mensaje8->pid = 2;
	mensaje8->id_segmento = 0;
	mensaje8->tamanio_segmento = 0;
	mensaje8->dir_fisica = 0;
	mensaje8->tamanio_buffer = 0;
	mensaje8->buffer = strdup("\0");
	mensaje8->origen_mensaje = KERNEL;
	list_push_con_mutex(lista_fifo_msj, mensaje8, &mutex_cola_msj);
	sem_post(&sem_cant_msj);

	return;
}

/* DEPRECATED
int asignar_id_segmento(int pid){
	nodoProceso* nodoP = buscar_por_pid(lista_procesos, pid);

	int i = 0;

	for( ; (i < atoi(lectura_de_config.CANT_SEGMENTOS)) && buscar_por_id(nodoP->lista_segmentos, i) != NULL ; i++)
	{
	}

	return i;
}
*/

/* DEPRECATED
int hay_seg_fault(int pid, int id_segmento, int dir_fisica, int tamanio_buffer) { //CHEQUEADA
	nodoProceso* nodoP = buscar_por_pid(lista_procesos, pid);
	nodoSegmento* nodoS = buscar_por_id(nodoP->lista_segmentos, id_segmento);

	if(dir_fisica < nodoS->base)
		return 1;

	if(dir_fisica + tamanio_buffer > nodoS->base + nodoS->tamanio)
		return 1;

	return 0;
}
*/
