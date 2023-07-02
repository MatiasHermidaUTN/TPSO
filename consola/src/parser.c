#include "../include/parser.h"

t_list* parsear_pseudocodigo(char* direccion_pseudocodigo) {
    FILE* archivo_pseudocodigo = fopen(direccion_pseudocodigo, "r"); // @suppress("Type cannot be resolved")
    if (!archivo_pseudocodigo) {
        log_error(logger, "Error al abrir archivo pseudocodigo");
        exit(EXIT_FAILURE);
    }

    t_list* instrucciones_leidas = list_create();
    char* linea_leida = NULL;
    size_t length = 0;

    while (getline(&linea_leida, &length, archivo_pseudocodigo) != -1) {
    	list_add(instrucciones_leidas, parsear_instruccion(linea_leida));
    }

	free(linea_leida);
    fclose(archivo_pseudocodigo);

    return instrucciones_leidas;
}

t_instruccion* parsear_instruccion(char* linea_leida) {
	char* nombre_instruccion = strtok(linea_leida, " \n");
    if (!nombre_instruccion) {
        log_error(logger, "Error: No se pudo leer la instruccion.");
        exit(EXIT_FAILURE);
    }

    //Agrega nombre de instrucción
	t_instruccion* instruccion_a_parsear = malloc(sizeof(t_instruccion));

	instruccion_a_parsear->nombre = strdup(nombre_instruccion);

	instruccion_a_parsear->parametros = list_create();

	//Agrega parámetros
	char* token = strtok(NULL, " \n");

	char* parametro_leido = NULL;
	if(token) {
		parametro_leido = strdup(token);
	}

	while(token) {
		list_add(instruccion_a_parsear->parametros, parametro_leido);

		token = strtok(NULL, " \n");
		if(token) {
			parametro_leido = strdup(token);
		}
	}

    //key: instruccion; element: cantidad de parámetros
	t_dictionary* diccionario_instrucciones = crear_diccionario_instrucciones();

	//Verifica que la cantidad de parametros sea correcta
	int cantidad_de_parametros_obtenidos = list_size(instruccion_a_parsear->parametros);
	int* cantidad_de_parametros_esperados = dictionary_get(diccionario_instrucciones, nombre_instruccion);
	if(*cantidad_de_parametros_esperados != cantidad_de_parametros_obtenidos) {
		log_error(logger, "Error: %s esperaba %d parametros, pero recibió %d.", nombre_instruccion, *cantidad_de_parametros_esperados, cantidad_de_parametros_obtenidos);
	}

    dictionary_destroy_and_destroy_elements(diccionario_instrucciones, (void*)free);

	return instruccion_a_parsear;
}

