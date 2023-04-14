#ifndef CONSOLA_H_
#define CONSOLA_H_

#include <utils.h>
#include <commons/string.h>
#include <commons/config.h>
#include <commons/log.h>
#include <commons/collections/dictionary.h>
#include <commons/collections/list.h>
#include <stdio.h>
#include "configuracion_consola.h" // Fijarse si está bien
#include "parser.h"
#include "comunicaciones_consola.h"

void destruir_instruccion(t_instruccion* instruccion);
void destruir_parametro(char* parametro);

#endif /* CONSOLA_H_ */
