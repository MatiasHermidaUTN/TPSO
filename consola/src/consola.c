#include <utils.h>
#include <commons/string.h>
#include <commons/config.h>
#include <commons/log.h>
#include "../include/configuracion_consola.h" // Fijarse si está bien

#define CANT_INSTRUCCIONES 16 //se puede sacar si usamos size?

typedef nombreInstrucciones = char[16];

nombreInstrucciones cod_op[CANT_INSTRUCCIONES] = ["F_READ", "F_WRITE", "SET", "MOVE_IN", "I/O", "WAIT", "SIGNAL", "F_OPEN", "F_CLOSE", "DELETE_SEGMENT", "MOV_OUT", "F_TRUNCATE", "F_SEEK", "CREATE_SEGMENT", "EXIT", "YIELD"];
int cant_params[CANT_INSTRUCCIONES] = [3, 3, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 0, 0];

typedef struct instruccion {
    int codOp;
    char** parametros;
} t_instruccion;


int main(int argc, char** argv) {
    t_config* config = iniciar_config("consola.config");
    t_log* logger    = iniciar_logger("consola.log", "proceso");

    t_consola_config lectura_de_config = leer_consola_config(config);


	log_info(logger, "%s", lectura_de_config.IP_KERNEL);//el %s es para que no tire warning para tomarlo como literal cadena
	log_info(logger, "%s", lectura_de_config.PUERTO_KERNEL);

    parsearPseudocodigo(logger, argv[1]);

	int conexion = crear_conexion(lectura_de_config.IP_KERNEL, lectura_de_config.PUERTO_KERNEL);

    ////// Enviamos al servidor el valor de CLAVE como mensaje
    puts("antes de mandar");

    enviar_mensaje("mensaje1", conexion);
    enviar_mensaje("mensaje2", conexion);

    puts("despues de mandar");
	//terminar_programa(conexion, logger, config);
}


void parsearPseudocodigo(t_log* logger, char* direccionPseudocodigo) {
    FILE * archivoPseudocodigo;
    char * instruccionLeida = NULL;
    int length = 0;
    int read;

    archivoPseudocodigo = fopen(direccionPseudocodigo, "r");
    if (!archivoPseudocodigo) {
        log_error(logger, "Error al abrir archivo pseudocodigo");
        exit(EXIT_FAILURE); 
    }

    while ((read = getline(&instruccionLeida, &length, archivoPseudocodigo)) != -1) {
        char** instruccionParseada = string_split(instruccionLeida, " ");
        t_instruccion instruccion;
        instruccion.codOp = obtenerCodOp(instruccionParseada[0]);
        obtenerParametros(&instruccion.parametros, instruccionParseada);
        if (cant_params[instruccion.codOp] != string_array_size(instruccion.parametros))
            log_error(logger, "La instruccion esperaba mas/menos parametros");

        //puts(instruccion);
    }

    fclose(archivoPseudocodigo);
}

int obtenerCodOp(char* codigo_str){
    for(int i = 0 ; i < CANT_INSTRUCCIONES ; i++){
        if(! strcmp(cod_op[i], codigo_str)) return i; 
    }
    log_error(logger, "La instruccion no es valida");
    exit(EXIT_FAILURE); //rompe programa, se podria hacer que no le de importancia a la linea
}

void obtenerParametros(char** parametros, char** instruccionParseada, int cantParametros){
    int cantParametros = string_array_size(instruccionParseada) - 1;
    for(int i = 0; i < cantParametros ; i++){
        parametros[i] = instruccionParseada[i + 1];
    }
}