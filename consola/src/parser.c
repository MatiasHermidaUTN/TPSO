#include "../include/parser.h"

t_list* parsearPseudocodigo(t_log* logger, char* direccionPseudocodigo) {
    FILE * archivoPseudocodigo;
    char * lineaLeida = NULL;
    size_t length = 0;
    int read;

    // *** ABRIR ARCHIVO CON INSTRUCCIONES *** //
    archivoPseudocodigo = fopen(direccionPseudocodigo, "r");
    if (!archivoPseudocodigo) {
        log_error(logger, "Error al abrir archivo pseudocodigo");
        exit(EXIT_FAILURE);
    }
    // *** DICCIONARIO CON INSTRUC + CANT PARAM *** //
	t_dictionary* instrucciones = crear_diccionario_instrucciones();

	// *** LEO INSTRUC Y AGREGO A LISTA *** //
    t_list* instruccionesLeidas = list_create();
    while ((read = getline(&lineaLeida, &length, archivoPseudocodigo)) != -1) {
    	t_instruccion* instruccionParseada = parsearInstruccion(lineaLeida, logger, instrucciones);
    	list_add(instruccionesLeidas, instruccionParseada);
    }
	free(lineaLeida);

    fclose(archivoPseudocodigo);

    dictionary_destroy_and_destroy_elements(instrucciones, (void*)destruir_diccionario);

    return instruccionesLeidas;
}

t_instruccion* parsearInstruccion(char* lineaLeida, t_log* logger, t_dictionary* instrucciones) {
	t_instruccion* instruccionAParsear = malloc(sizeof(t_instruccion));
	char* nombreInstruccion = strtok(lineaLeida, " \n");
	char* parametroLeido = NULL;
	char* token;

    if (!nombreInstruccion) {
        log_error(logger, "Error: No se pudo leer la instruccion.");
        exit(EXIT_FAILURE);
    }
    //Agrega nombre de instrucción
	instruccionAParsear->nombre = strdup(nombreInstruccion);
	printf("%s: ", instruccionAParsear->nombre);

	instruccionAParsear->parametros = list_create();

	//Agrega parámetros
	token = strtok(NULL, " \n");
	if(token) parametroLeido = strdup(token);
	while(token) {
		list_add(instruccionAParsear->parametros, parametroLeido);
		printf("%s ", parametroLeido);
		token = strtok(NULL, " \n");
		if(token) parametroLeido = strdup(token);
	}

	//Verifica que la cantidad de parametros sea correcta
	int cantidadDeParametrosObtenidos = list_size(instruccionAParsear->parametros);
	int* cantidadDeParametrosEsperados = (int*)dictionary_get(instrucciones, nombreInstruccion);
	if(*cantidadDeParametrosEsperados != cantidadDeParametrosObtenidos) {
		log_error(logger, "Error: %s esperaba %d parametros, pero recibio %d.", nombreInstruccion, *cantidadDeParametrosEsperados, cantidadDeParametrosObtenidos);
	}
	puts("\n");

	return instruccionAParsear;
}

t_dictionary* crear_diccionario_instrucciones(){
	t_dictionary* instrucciones = dictionary_create();
	agregar_a_diccionario(instrucciones, "F_READ",         3);
	agregar_a_diccionario(instrucciones, "F_WRITE",        3);
	agregar_a_diccionario(instrucciones, "SET",            2);
	agregar_a_diccionario(instrucciones, "MOV_IN",         2);
	agregar_a_diccionario(instrucciones, "MOV_OUT",        2);
	agregar_a_diccionario(instrucciones, "F_TRUNCATE", 	   2);
	agregar_a_diccionario(instrucciones, "F_SEEK",    	   2);
	agregar_a_diccionario(instrucciones, "CREATE_SEGMENT", 2);
	agregar_a_diccionario(instrucciones, "I/O",            1);
	agregar_a_diccionario(instrucciones, "WAIT",           1);
	agregar_a_diccionario(instrucciones, "SIGNAL",         1);
	agregar_a_diccionario(instrucciones, "F_OPEN",         1);
	agregar_a_diccionario(instrucciones, "F_CLOSE",        1);
	agregar_a_diccionario(instrucciones, "DELETE_SEGMENT", 1);
	agregar_a_diccionario(instrucciones, "EXIT",           0);
	agregar_a_diccionario(instrucciones, "YIELD",          0);
	return instrucciones;
}

void agregar_a_diccionario(t_dictionary* diccionario, char* key, int elemento){
	int* elemento_a_agreagar = malloc(sizeof(elemento));
	*elemento_a_agreagar = elemento;
	dictionary_put(diccionario, key, elemento_a_agreagar);
}

void destruir_diccionario(int* cantParametros){
	free(cantParametros);
}
