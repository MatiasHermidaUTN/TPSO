#include <utils.h>
#include <commons/string.h>
#include <commons/config.h>
#include <commons/log.h>
#include "../include/configuracion_consola.h" // Fijarse si está bien
#include <commons/collections/dictionary.h>
#include <commons/collections/list.h>

typedef struct instruccion {
    char* nombre;
    t_list* parametros;
} t_instruccion;

t_list* parsearPseudocodigo(t_log* logger, char* direccionPseudocodigo);
t_instruccion* parsearInstruccion(char* lineaLeida, t_log* logger, t_dictionary*);
void destruir_instruccion(t_instruccion* instruccion);
void destruir_parametro(char* parametro);
void destruir_diccionario(int* cantParametros);
void agregar_a_diccionario(t_dictionary* diccionario, char* key, int elemento);

int main(int argc, char** argv) {
    t_config* config = iniciar_config("../consola.config");
    t_log* logger    = iniciar_logger("consola.log", "proceso");

    t_consola_config lectura_de_config = leer_consola_config(config);


	log_info(logger, "%s", lectura_de_config.IP_KERNEL);//el %s es para que no tire warning para tomarlo como literal cadena
	log_info(logger, "%s", lectura_de_config.PUERTO_KERNEL);

    t_list* instrucciones = parsearPseudocodigo(logger, argv[1]);

	int socket_kernel = crear_conexion(lectura_de_config.IP_KERNEL, lectura_de_config.PUERTO_KERNEL);

    list_destroy_and_destroy_elements(instrucciones, (void *)destruir_instruccion);
    log_destroy(logger);
    config_destroy(config);
    liberar_estructura_config(lectura_de_config);
    close(socket_kernel);

	//terminar_programa(conexion, logger, config);
}


t_list* parsearPseudocodigo(t_log* logger, char* direccionPseudocodigo) {
    FILE * archivoPseudocodigo;
    char * lineaLeida = NULL;
    size_t length = 0;
    int read;

    archivoPseudocodigo = fopen(direccionPseudocodigo, "r");
    if (!archivoPseudocodigo) {
        log_error(logger, "Error al abrir archivo pseudocodigo");
        exit(EXIT_FAILURE); 
    }
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



    t_list* instruccionesLeidas = list_create();
    while ((read = getline(&lineaLeida, &length, archivoPseudocodigo)) != -1) {
    	t_instruccion* instruccionParseada = parsearInstruccion(lineaLeida, logger, instrucciones);
    	list_add(instruccionesLeidas, instruccionParseada);
		puts("");
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

		log_info(logger,"%s ", parametroLeido);
		token = strtok(NULL, " \n");
		if(token) parametroLeido = strdup(token);
	}

	puts("");

	//Verifica que la cantidad de parametros sea correcta
	int cantidadDeParametrosObtenidos = list_size(instruccionAParsear->parametros);
	int* cantidadDeParametrosEsperados = (int*)dictionary_get(instrucciones, nombreInstruccion);
	if(*cantidadDeParametrosEsperados != cantidadDeParametrosObtenidos) {
		log_error(logger, "Error: %s esperaba %d parametros, pero recibio %d.", nombreInstruccion, *cantidadDeParametrosEsperados, cantidadDeParametrosObtenidos);
	}

	return instruccionAParsear;
}

void destruir_parametro(char* parametro){
	free(parametro);
}

void destruir_instruccion(t_instruccion* instruccion){
	free(instruccion->nombre);
	list_destroy_and_destroy_elements(instruccion->parametros, (void*)destruir_parametro);
	free(instruccion);
}

void destruir_diccionario(int* cantParametros){
	free(cantParametros);
}

void agregar_a_diccionario(t_dictionary* diccionario, char* key, int elemento){
	int* elemento_a_agreagar = malloc(sizeof(elemento));
	*elemento_a_agreagar = elemento;
	dictionary_put(diccionario, key, elemento_a_agreagar);
}
