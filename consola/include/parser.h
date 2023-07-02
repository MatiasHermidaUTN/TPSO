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
#include "configuracion_consola.h"

t_list* parsear_pseudocodigo(char* direccion_pseudocodigo);
t_instruccion* parsear_instruccion(char* linea_leida);

#endif /* PARSER_H_ */
