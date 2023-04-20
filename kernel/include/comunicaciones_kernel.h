#ifndef COMUNICACION_H_
#define COMUNICACION_H_

#include <stdio.h>
#include <string.h>
#include <commons/string.h>
#include <commons/collections/list.h>

#include <diccionario_instrucciones.h>
#include <utils.h>
#include "planificacion_corto.h"

t_list* deserializar_instrucciones(void* a_recibir, int size_payload);
t_list* recibir_instrucciones(int socket_consola);
void deserializar_parametros(void* a_recibir, int* desplazamiento, t_instruccion* instruccion, t_dictionary* diccionario_instrucciones);
void print_l_instrucciones(t_list* instrucciones);

void enviar_pcb(t_pcb* pcb, int socket_cpu);
void* serializar_pcb(t_pcb* pcb, size_t* size_total);

size_t tamanio_payload_pcb(t_pcb* pcb);
size_t tamanio_instrucciones(t_list* instrucciones);
int tamanio_parametros(t_list* parametros, int index_instruccion);

void memcpy_instrucciones(void* stream, t_list* instrucciones, int* desplazamiento);
void memcpy_registros(void* stream, t_registros_cpu registros_cpu, int* desplazamiento);
void memcpy_tabla_segmentos(void* stream, t_list* tabla_segmentos, int* desplazamiento);
void memcpy_archivos_abiertos(void* stream, t_list* archivos_abiertos, int* desplazamiento);

t_rta_cpu_al_kernel esperar_cpu();

#endif /* COMUNICACION_H_ */
