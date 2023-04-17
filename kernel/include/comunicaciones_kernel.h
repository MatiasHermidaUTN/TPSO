#ifndef COMUNICACION_H_
#define COMUNICACION_H_

#include <utils.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include <stdio.h>
#include <diccionario_instrucciones.h>
#include "planificacion_corto.h"

typedef struct {
	char* nombre;
	t_list* parametros;
} t_instruccion;

t_list* deserializar_instrucciones(void* a_recibir, int size_payload);
t_list* recibir_instrucciones(int socket_consola);
void deserializar_parametros(void* a_recibir, int* desplazamiento, t_instruccion* instruccion, t_dictionary* diccionario_instrucciones);
void print_l_instrucciones(t_list* instrucciones);
void enviar_pcb(t_pcb* pcb);
t_rta_cpu_al_kernel esperar_cpu();

#endif /* COMUNICACION_H_ */
