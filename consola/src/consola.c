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

void parsearPseudocodigo(t_log* logger, char* direccionPseudocodigo);
t_instruccion* parsearInstruccion(char* lineaLeida, t_log* logger, t_dictionary*);

int main(int argc, char** argv) {
    t_config* config = iniciar_config("../consola.config");
    t_log* logger    = iniciar_logger("consola.log", "proceso");

    t_consola_config lectura_de_config = leer_consola_config(config);


	log_info(logger, "%s", lectura_de_config.IP_KERNEL);//el %s es para que no tire warning para tomarlo como literal cadena
	log_info(logger, "%s", lectura_de_config.PUERTO_KERNEL);

    parsearPseudocodigo(logger, argv[1]);

	int conexion = crear_conexion(lectura_de_config.IP_KERNEL, lectura_de_config.PUERTO_KERNEL);

    ////// Enviamos al servidor el valor de CLAVE como mensaje

    enviar_mensaje("mensaje1", conexion);
    enviar_mensaje("mensaje2", conexion);

	//terminar_programa(conexion, logger, config);
}


void parsearPseudocodigo(t_log* logger, char* direccionPseudocodigo) {
    FILE * archivoPseudocodigo;
    char * lineaLeida = NULL;
    size_t length = 0;
    int read;

    archivoPseudocodigo = fopen(direccionPseudocodigo, "r");
    if (!archivoPseudocodigo) {
        log_error(logger, "Error al abrir archivo pseudocodigo");
        exit(EXIT_FAILURE); 
    }
    puts("Abrio el archivo\n");

	t_dictionary* instrucciones = dictionary_create();

	dictionary_put(instrucciones, "F_READ",         (void*)3);
	dictionary_put(instrucciones, "F_WRITE",        (void*)3);
	dictionary_put(instrucciones, "SET",            (void*)2);
	dictionary_put(instrucciones, "MOV_IN",         (void*)2);
	dictionary_put(instrucciones, "MOV_OUT",        (void*)2);
	dictionary_put(instrucciones, "F_TRUNCATE",     (void*)2);
	dictionary_put(instrucciones, "F_SEEK",         (void*)2);
	dictionary_put(instrucciones, "CREATE_SEGMENT", (void*)2);
	dictionary_put(instrucciones, "I/O",            (void*)1);
	dictionary_put(instrucciones, "WAIT",           (void*)1);
	dictionary_put(instrucciones, "SIGNAL",         (void*)1);
	dictionary_put(instrucciones, "F_OPEN",         (void*)1);
	dictionary_put(instrucciones, "F_CLOSE",        (void*)1);
	dictionary_put(instrucciones, "DELETE_SEGMENT", (void*)1);
	dictionary_put(instrucciones, "EXIT",           (void*)0);
	dictionary_put(instrucciones, "YIELD",          (void*)0);



    t_list* instruccionesLeidas = list_create();
    while ((read = getline(&lineaLeida, &length, archivoPseudocodigo)) != -1) {
		list_add(instruccionesLeidas, parsearInstruccion(lineaLeida, logger, instrucciones));
		puts("");
    }
	free(lineaLeida);

    fclose(archivoPseudocodigo);
}

t_instruccion* parsearInstruccion(char* lineaLeida, t_log* logger, t_dictionary* instrucciones) {
	t_instruccion* instruccionAParsear = malloc(sizeof(t_instruccion));
	char* nombreInstruccion = strtok(lineaLeida, " \n");
	char* parametroLeido;

    if (!nombreInstruccion) {
        log_info(logger, "Error: No se pudo leer la instruccion.");
        exit(EXIT_FAILURE);
    }
    //Agrega nombre de instrucción
	instruccionAParsear->nombre = strdup(nombreInstruccion);
	printf("%s: ", instruccionAParsear->nombre);

	instruccionAParsear->parametros = list_create();

	//Agrega parámetros
	parametroLeido = strtok(NULL, " \n");
	while(parametroLeido) {
		list_add(instruccionAParsear->parametros, parametroLeido);
		printf("%s ", parametroLeido);

		parametroLeido = strtok(NULL, " \n");
	}

	puts("");

	//Verifica que la cantidad de parámetros sea correcta
	int cantidadDeParametrosObtenidos = list_size(instruccionAParsear->parametros);
	int cantidadDeParametrosEsperados = (int)(intptr_t)dictionary_get(instrucciones, nombreInstruccion);
	if(cantidadDeParametrosEsperados != cantidadDeParametrosObtenidos) {
		log_info(logger, "Error: %s esperaba %d parametros, pero recibió %d.", nombreInstruccion, cantidadDeParametrosEsperados, cantidadDeParametrosObtenidos);
	}

	return instruccionAParsear;
}
