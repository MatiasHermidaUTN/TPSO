#ifndef MEMORIA_H_
#define MEMORIA_H_


#include <commons/bitarray.h>

#include "configuracion_memoria.h"
#include "comunicaciones_memoria.h"

typedef struct nodoSegmento {
	int id;
	int base;
	int tamanio;
} nodoSegmento;

typedef struct nodoProceso {
	int pid;
	t_list* lista_segmentos;
} nodoProceso;

eliminar_lista_procesos
//MEMORIA
int manejar_mensaje();
void string_array_push_para_lista_segmentos(char ***parametros_a_enviar, t_list* lista_segmentos);
void compactar();
void eliminar_segmento(int pid, int id_segmento);
void eliminar_proceso(int pid);
int tengo_espacio_contiguo(int tamanio_segmento);
int tengo_espacio_general(int tamanio_segmento);
nodoProceso* crear_proceso(int pid);
int crear_segmento(int pid, int id_segmento, int tamanio_segmento);
int asignar_espacio_en_memoria(int tamanio_segmento);
void hay_seg_fault(int pid, int id_segmento, int dir_fisica, int tamanio_buffer);

//ALGORITMOS LISTAS
nodoProceso* buscar_por_pid(int pid);
nodoSegmento* buscar_por_id(t_list* lista_segmentos, int id_segmento);
void buscar_pid_y_id_segmento_por_base(int base, int* pid, int* id_segmento);
void buscar_pid_y_id_segmento_por_dir_fisica(int dir_fisica, int* pid, int* id_segmento);
void eliminar_lista_procesos();
void eliminar_lista_mensajes();
void log_compactacion();

#endif /* MEMORIA_H_ */

