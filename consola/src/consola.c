#include <utils.h>
#include <commons/string.h>
#include <commons/config.h>
#include <commons/log.h>
#include "../include/configuracion_consola.h" // Fijarse si está bien

#define CANT_INSTRUCCIONES 16 //se puede sacar si usamos size?

//typedef nombreInstrucciones = char[16];

const char* cod_op[CANT_INSTRUCCIONES] = {"F_READ", "F_WRITE", "SET", "MOV_IN", "MOV_OUT", "F_TRUNCATE", "F_SEEK", "CREATE_SEGMENT", "I/O", "WAIT", "SIGNAL", "F_OPEN", "F_CLOSE", "DELETE_SEGMENT", "EXIT", "YIELD"};
const int cant_params[CANT_INSTRUCCIONES] = {3, 3, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 0, 0};

typedef struct instruccion {
    int codOp;
    char** parametros;
} t_instruccion;

void parsearPseudocodigo(t_log* logger, char* direccionPseudocodigo);
int obtenerCodOp(t_log* logger,char* codigo_str);
void obtenerParametros(char*** parametros, char** instruccionParseada);
void liberar(t_instruccion *instruccion,int cantParams);

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
    char * instruccionLeida = NULL;
    size_t length = 0;
    int read;

    archivoPseudocodigo = fopen(direccionPseudocodigo, "r");
    if (!archivoPseudocodigo) {
        log_error(logger, "Error al abrir archivo pseudocodigo");
        exit(EXIT_FAILURE); 
    }
    puts("abrio el archivo");

    while ((read = getline(&instruccionLeida, &length, archivoPseudocodigo)) != -1) {
    	instruccionLeida = string_replace(instruccionLeida,"\n","\0");
		char** instruccionParseada = string_split(instruccionLeida, " ");

        t_instruccion instruccion;
        instruccion.codOp = obtenerCodOp(logger,instruccionParseada[0]);
        puts(instruccionParseada[0]);
        obtenerParametros(&(instruccion.parametros), instruccionParseada);

        for(int i=0;i<string_array_size(instruccionParseada);i++){
        	//puts(instruccionParseada[i]);
        }
    	puts("");


        if (cant_params[instruccion.codOp] != string_array_size(instruccion.parametros))
            log_error(logger, "%s esperaba %d parametros, pero recibio %d paremtros",instruccionParseada[0],cant_params[instruccion.codOp], string_array_size(instruccion.parametros));

        liberar(&instruccion, cant_params[instruccion.codOp]);

    }

    fclose(archivoPseudocodigo);
}

int obtenerCodOp(t_log* logger,char* codigo_str){
    for(int i = 0 ; i < CANT_INSTRUCCIONES ; i++){
        if(! strcmp(cod_op[i], codigo_str)) return i; 
    }
    log_error(logger, "La instruccion no es valida");
    exit(EXIT_FAILURE); //rompe programa, se podria hacer que no le de importancia a la linea
}

void obtenerParametros(char*** parametros, char** instruccionParseada){
    int cantParametros = string_array_size(instruccionParseada) - 1;
	*parametros = malloc(sizeof(char*)* cantParametros);

    for(int i = 0; i < cantParametros ; i++){
        (*parametros)[i] = strdup(instruccionParseada[i + 1]);
    }
}

void liberar(t_instruccion *instruccion,int cantParams){
	for(int i=0;i<cantParams;i++){
		free((*instruccion).parametros[i]);
	}
	free(instruccion->parametros);
}
