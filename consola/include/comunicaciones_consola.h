#ifndef COMUNICACION_H_
#define COMUNICACION_H_

#include <utils.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include <stdio.h>
#include <utils.h>
#include "parser.h"

void* serializar_instrucciones(t_list* instrucciones, int size);
int enviar_instrucciones(t_list* instrucciones);
int tamanio_msj(t_list* instrucciones);
int calculo_tamanio_parametros(t_list* parametros, int index_instruccion);


#endif /* COMUNICACION_H_ */
