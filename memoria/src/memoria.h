#ifndef MEMORIA_H_
#define MEMORIA_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include <math.h>
#include <time.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <commons/bitarray.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>
#include <utils.h>
#include "../include/configuracion_memoria.h"
#include "../include/conexiones_memoria.h"

typedef struct nodoSegmento {
	int id;
	int base;
	int tamanio;
	struct nodoSegmento* siguiente;
} nodoSegmento;

typedef struct nodoProceso {
	int pid;
	struct nodoSegmento* lista_segmentos;
	struct nodoProceso* siguiente;
} nodoProceso;

typedef enum
{
	INICIALIZAR_EL_PROCESO,
	CREAR_SEGMENTO,
	ELIMINAR_SEGMENTO,
	ESCRIBIR,
	LEER,
	COMPACTAR,
	ELIMINAR_PROCESO,
	ERROR,
} t_instrucciones;

typedef enum
{
	FIRST,
	BEST,
	WORST,
} t_algoritmo_asignacion;

void manejar_mensaje(int cod_op, int pid, int id_segmento, int tamanio_segmento, int dir_fisica, char* buffer, int tamanio_buffer, char* origen_mensaje);
void compactar();
int hay_seg_fault(int pid, int id_segmento, int dir_fisica, int tamanio_buffer);
void eliminar_segmento(int pid, int id_segmento);
int tengo_espacio_contiguo(int tamanio_segmento);
int tengo_espacio_general(int tamanio_segmento);
nodoProceso* crear_proceso(int pid);
int crear_segmento(int pid, int id_segmento, int tamanio_segmento);
int asignar_espacio_en_memoria(int tamanio_segmento);
nodoProceso* buscar_por_pid(nodoProceso* lista_procesos, int pid);
nodoSegmento* buscar_por_id(nodoSegmento* lista_segmentos, int id_segmento);
void borrar_nodo_segmento(nodoSegmento** referenciaLista, nodoSegmento* nodo_a_borrar);
int length_segmento(nodoSegmento* nodoS);
void buscar_por_base(nodoProceso* nodoP, int base, int* pid, int* id_segmento);
void push_segmento(nodoSegmento** referencia_lista, nodoSegmento* nodoS);
void push_proceso(nodoProceso** referencia_lista, nodoProceso* nodoP);
void eliminar_lista_proceso(nodoProceso** lista_procesos);

void imprimir_bitmap();
void imprimir_lista();
void imprimir_memoria(int desde, int hasta);

#endif /* MEMORIA_H_ */
