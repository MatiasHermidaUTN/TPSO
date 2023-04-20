#ifndef COMUNICACION_H_
#define COMUNICACION_H_

#include <utils.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include <stdio.h>
#include <diccionario_instrucciones.h>
#include "planificacion_corto.h"


t_list* deserializar_instrucciones_kernel(void* a_recibir, int size_payload);
t_list* recibir_instrucciones(int socket_consola);
void deserializar_parametros(void* a_recibir, int* desplazamiento, t_instruccion* instruccion, t_dictionary* diccionario_instrucciones);
void print_l_instrucciones(t_list* instrucciones);
t_msj_kernel_cpu esperar_cpu();

#endif /* COMUNICACION_H_ */
