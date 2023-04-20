#ifndef CICLO_INSTRUCCION_H_
#define CICLO_INSTRUCCION_H_

#include <stdio.h>
#include <stdlib.h>

#include <utils.h>
#include <diccionario_instrucciones.h>

void correr_ciclo_instruccion(int socket_kernel);

t_pcb* recibir_pcb(int socket_kernel);

t_pcb* deserializar_pcb(void* stream, size_t size_payload);

t_list* deserializar_instrucciones(void* a_recibir, size_t size_payload, int* desplazamiento);

void deserializar_parametros(void* a_recibir, int* desplazamiento, t_instruccion* instruccion, t_dictionary* diccionario_instrucciones);

void print_l_instrucciones(t_list* instrucciones);

void memcpy_registros(t_registros_cpu* registros_cpu, void* stream_pcb, int* desplazamiento);

void memcpy_tabla_segmentos(t_list* tabla_segmentos, void* stream, int* desplazamiento);

void memcpy_archivos_abiertos(t_list* archivos_abiertos, void* stream, int* desplazamiento);

#endif /* CICLO_INSTRUCCION_H_ */
