#include "../include/memoria.h"

nodoProceso* lista_procesos;
//MEMORIA PRINCIPAL
void* memoria_principal;

void* bitmap_pointer;
t_bitarray* bitarray_de_bitmap;
////////////////////////////////////////////////////////////

t_memoria_config lectura_de_config;

int socket_memoria;
/*
void sighandler(int x) {
    switch (x) {
        case SIGINT:
        	close(socket_memoria);
            exit(EXIT_SUCCESS);
    }
}*/

int main(int argc, char** argv) {
	//signal(SIGINT, sighandler);
	t_config* config = iniciar_config(argv[1]);
	lectura_de_config = leer_memoria_config(config);
    t_log* logger = iniciar_logger("memoria.log", "Memoria");

    //socket_memoria = iniciar_servidor("127.0.0.1", lectura_de_config.PUERTO_ESCUCHA); //preguntar si la ip tiene que estar en el config o no
	//log_warning(logger, "Memoria lista para recibir al cliente");

	//while(recibir_conexiones(socket_memoria, logger)); //Recibe conexiones y crea hilos para manejarlas

	memoria_principal = malloc(atoi(lectura_de_config.TAM_MEMORIA));
	memset(memoria_principal, 0, atoi(lectura_de_config.TAM_MEMORIA));

	lista_procesos = NULL;

	//BITMAP
	int tamanio_bitmap = (int)(atoi(lectura_de_config.TAM_MEMORIA) / 8);
	bitmap_pointer = malloc(tamanio_bitmap); //la division tiene q dar entera
	if(!(atoi(lectura_de_config.TAM_MEMORIA) % 8)){
		//error
	}
	bitarray_de_bitmap = bitarray_create_with_mode(bitmap_pointer, tamanio_bitmap, MSB_FIRST);

	log_info(logger, "Bitmap creado");

	//Segmento 0
	for(int i = 0; i < atoi(lectura_de_config.TAM_SEGMENTO_0); i++){
		bitarray_set_bit(bitarray_de_bitmap, i);
	}
	log_info(logger, "segmento 0 creado en el bitmap");

	//TODO test
	for(int i = atoi(lectura_de_config.TAM_SEGMENTO_0)+1; i < atoi(lectura_de_config.TAM_SEGMENTO_0)+1+3; i++){
		bitarray_set_bit(bitarray_de_bitmap, i);
	}
	for(int i = atoi(lectura_de_config.TAM_SEGMENTO_0)+1+3+5; i < atoi(lectura_de_config.TAM_SEGMENTO_0)+1+3+5+7; i++){
		bitarray_set_bit(bitarray_de_bitmap, i);
	}
	for(int i = atoi(lectura_de_config.TAM_SEGMENTO_0)+1+3+5+7+4; i < atoi(lectura_de_config.TAM_SEGMENTO_0)+1+3+5+7+4+2; i++){
		bitarray_set_bit(bitarray_de_bitmap, i);
	}

	printf("\n");
	imprimir_bitmap();
	printf("\n");
	//TODO test

	char* origen = strdup("kernel");
	char* buffer1 = strdup("");
	manejar_mensaje(INICIALIZAR_EL_PROCESO, 2, 0, 0, 0, buffer1, 0, origen);
	printf("inicializado\n");

	manejar_mensaje(CREAR_SEGMENTO, 2, 1, 4, 0, buffer1, 0, origen);
	printf("creado\n");

	printf("\n");
	imprimir_bitmap();
	printf("\n");

	log_destroy(logger);
	config_destroy(config);
	free(memoria_principal);
	eliminar_lista_proceso(&lista_procesos);

	return EXIT_SUCCESS;
}


void manejar_mensaje(int cod_op, int pid, int id_segmento, int tamanio_segmento, int dir_fisica, char* buffer, int tamanio_buffer, char* origen_mensaje) { //Prueba
	/*
	t_instruccion cod_op;
	int pid;
	int id_segmento;
	int tamanio_segmento;
	int dir_fisica;
	char* buffer;
	int tamanio_buffer;
	char* origen_mensaje;*/

	switch (cod_op) {
		case INICIALIZAR_EL_PROCESO:{
			//recibir_parametros(pid, origen_mensaje); 	//TODO
			nodoProceso* nodoP = crear_proceso(pid);
			//msj kernel proceso creado y tabla actualizada

			free(origen_mensaje);
			break;
		}
		case CREAR_SEGMENTO:{
			//recibir_parametros(pid, id_segmento, tamanio_segmento, origen_mensaje);
			if(!tengo_espacio_general(tamanio_segmento)) {
				//error?
			}
			if(!tengo_espacio_contiguo(tamanio_segmento)) {
				//msj kernel compactacion
			}
			int base = crear_segmento(pid, id_segmento, tamanio_segmento); //TODO definir id_segmento yo
			//msj kernel segmento creado y base

			free(origen_mensaje);
			break;
		}
		case ELIMINAR_SEGMENTO:{
			//recibir_parametros(pid, id_segmento, origen_mensaje);

			eliminar_segmento(pid, id_segmento);

			nodoProceso* nodoP = buscar_por_pid(lista_procesos, pid);

			//msj kernel segmento eliminado y tabla actualizada

			free(buffer);
			free(origen_mensaje);
			free(nodoP);

			break;
		}
		case ESCRIBIR:{
			//recibir_parametros(pid, id_segmento, dir_fisica, buffer, tamanio_buffer, origen_mensaje);
			if(hay_seg_fault(pid, id_segmento, dir_fisica, tamanio_buffer)){
				//msj kernel seg fault
			}

			memcpy(memoria_principal + dir_fisica, buffer, tamanio_buffer);

			//msj kernel escrito

			free(buffer);
			free(origen_mensaje);
			break;
		}
		case LEER:{
			//recibir_parametros(pid, id_segmento, dir_fisica, buffer, tamanio_buffer, origen_mensaje);
			if(hay_seg_fault(pid, id_segmento, dir_fisica, tamanio_buffer)){
				//msj kernel seg fault
			}

			memcpy(buffer, memoria_principal + dir_fisica, tamanio_buffer);

			//enviar buffer

			free(buffer);
			free(origen_mensaje);
			break;
		}
		case COMPACTAR:{
			compactar();

			//msj kernel compactacion done

			free(buffer);
			free(origen_mensaje);

			break;
		}
		case ERROR:{ //revisar
			//recibir_parametros(pid, id_segmento, dir_fisica, buffer, tamanio_buffer, origen_mensaje, etc);

			//msj kernel error

			free(buffer);
			free(origen_mensaje);

			break;
		}
		default:{ //revisar
			free(buffer);
			free(origen_mensaje);
			break;
		}
	}
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

					free(nodoP);
					free(nodoS);
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

	if(dir_fisica < nodoS->base) {
		free(nodoP);
		free(nodoS);
		return 1;
	}

	if(dir_fisica + tamanio_buffer > nodoS->base + nodoS->tamanio) {
		free(nodoP);
		free(nodoS);
		return 1;
	}

	free(nodoP);
	free(nodoS);
	return 0;
}

void eliminar_segmento(int pid, int id_segmento) { //CHEQUEADA
	nodoProceso* nodoP = buscar_por_pid(lista_procesos, pid);
	nodoSegmento* nodoS = buscar_por_id(nodoP->lista_segmentos, id_segmento);

	for(int i = nodoS->base; i < nodoS->base + nodoS->tamanio; i++){
		bitarray_clean_bit(bitarray_de_bitmap, i);
	}

	borrar_nodo_segmento(&(nodoP->lista_segmentos), nodoS);
	free(nodoP);
	free(nodoS);
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

int crear_segmento(int pid, int id_segmento, int tamanio_segmento) { //CHEQUEADA
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
	t_algoritmo_asignacion hola;
	if(!strcmp("FIRST", lectura_de_config.ALGORITMO_ASIGNACION))
		hola = FIRST;
	else if(!strcmp("BEST", lectura_de_config.ALGORITMO_ASIGNACION))
		hola = BEST;
	else if(!strcmp("WORST", lectura_de_config.ALGORITMO_ASIGNACION))
		hola = WORST;

	switch(hola){
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

//ALGORITMOS Listas

nodoProceso* buscar_por_pid(nodoProceso* lista_procesos, int pid) { //CHEQUEADA
	nodoProceso* nodoP = lista_procesos;

	while(nodoP->pid != pid){
		nodoP = nodoP->siguiente;
	}

	if(nodoP->pid != pid){
		return NULL; //va a romper cuando se intente usar
	}

	return nodoP;
}

nodoSegmento* buscar_por_id(nodoSegmento* lista_segmentos, int id_segmento) { //CHEQUEADA
	nodoSegmento* nodoS = lista_segmentos;

	while(nodoS->id != id_segmento){
		nodoS = nodoS->siguiente;
	}

	if(nodoS->id != id_segmento){
		return NULL; //va a romper cuando se intente usar
	}

	return nodoS;
}

void borrar_nodo_segmento(nodoSegmento** referenciaLista, nodoSegmento* nodo_a_borrar ) { //pinta bien
	nodoSegmento* nodo = *referenciaLista;
	nodoSegmento* nodoAnterior = NULL;
	while (nodo && (nodo != nodo_a_borrar))
	{
		nodoAnterior = nodo;
		nodo = nodo->siguiente;
	}
	if (nodo)
	{
		if (nodoAnterior)  // check principio de la lista
			nodoAnterior->siguiente = nodo->siguiente;
		else
			*referenciaLista = nodo->siguiente;

		free(nodo);
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
