#ifndef PARSER_H_
#define PARSER_H_

#include <utils.h>
#include <diccionario_instrucciones.h>
#include <commons/string.h>
#include <commons/config.h>
#include <commons/log.h>
#include <commons/collections/dictionary.h>
#include <commons/collections/list.h>
#include <stdio.h>

typedef struct {
	char* nombre;
	t_list* parametros;
} t_instruccion;

t_list* parsearPseudocodigo(t_log* logger, char* direccionPseudocodigo);
t_instruccion* parsearInstruccion(char* lineaLeida, t_log* logger, t_dictionary* instrucciones);

#endif /* PARSER_H_ */
