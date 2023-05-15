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
	ELIMINAR_PROCESO,
	ESCRIBIR,
	LEER,
	COMPACTAR,
	ERROR,
} t_instrucciones;

typedef enum
{
	FIRST,
	BEST,
	WORST,
} t_algoritmo_asignacion;

typedef enum
{
	KERNEL,
	CPU,
	FILESYSTEM,
} t_origen_mensaje;

typedef struct t_mensaje {
	t_instrucciones cod_op;
	int pid;
	int id_segmento;
	int tamanio_segmento;
	int dir_fisica;	
	int tamanio_buffer;
	char* buffer;
	t_origen_mensaje origen_mensaje;
} t_mensajes;

//MEMORIA
int manejar_mensaje();
void log_compactacion()
void compactar();
int hay_seg_fault(int pid, int id_segmento, int dir_fisica, int tamanio_buffer);
void eliminar_segmento(int pid, int id_segmento);
int tengo_espacio_contiguo(int tamanio_segmento);
int tengo_espacio_general(int tamanio_segmento);
nodoProceso* crear_proceso(int pid);
crear_segmento(int pid, int tamanio_segmento);
int asignar_espacio_en_memoria(int tamanio_segmento);
void asignar_id_segmento(int pid);

//ALGORITMOS LISTAS
nodoProceso* buscar_por_pid(nodoProceso* lista_procesos, int pid);
nodoSegmento* buscar_por_id(nodoSegmento* lista_segmentos, int id_segmento);
void borrar_nodo_segmento(nodoSegmento** referenciaListaSeg, nodoSegmento* nodo_a_borrar);
void borrar_nodo_proceso(nodoProceso** lista_procesos, nodoProceso* nodo_a_borrar);
int length_segmento(nodoSegmento* nodoS);
void buscar_por_base(nodoProceso* nodoP, int base, int* pid, int* id_segmento);
void push_segmento(nodoSegmento** referencia_lista, nodoSegmento* nodoS);
void push_proceso(nodoProceso** referencia_lista, nodoProceso* nodoP);
void eliminar_lista_proceso(nodoProceso** lista_procesos);

//PRUEBAS
void imprimir_bitmap();
void imprimir_lista();
void imprimir_memoria(int desde, int hasta);

#endif /* MEMORIA_H_ */
