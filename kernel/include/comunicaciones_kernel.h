#ifndef COMUNICACION_H_
#define COMUNICACION_H_

#include <utils.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include <stdio.h>
#include <diccionario_instrucciones.h>
#include "planificacion_corto.h"

t_list* recibir_instrucciones(int socket_consola);
t_msj_kernel_cpu esperar_cpu();
char** recibir_parametros_de_instruccion();

#endif /* COMUNICACION_H_ */
