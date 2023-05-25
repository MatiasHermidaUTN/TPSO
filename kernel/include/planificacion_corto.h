#ifndef PLANIFICACION_CORTO_H_
#define PLANIFICACION_CORTO_H_

#include <utils.h>
#include <pthread.h>
#include "configuracion_kernel.h"
#include "comunicaciones_kernel.h"

typedef struct {
	int tiempo;
	t_pcb* pcb;
} t_args_io;

void planificar_corto();

void ready_list_push(t_pcb* pcb_recibido);

void calcular_prox_rafaga(t_pcb* pcb);
int calcular_R(t_pcb* pcb);
int calcular_tiempo_en_ready(int segundos);

t_pcb* obtener_proximo_a_ejecutar();

void manejar_io(t_args_io* args_io);

void wait_recurso(t_pcb* pcb, char* recurso);
t_recurso* buscar_recurso(char* nombre_recurso, t_list* lista);
void signal_recurso(t_pcb* pcb, char* recurso);
void exit_proceso(t_pcb* pcb);
void list_remove_pcb(t_list *lista, t_pcb *pcb);
t_pcb* list_get_max_R(t_list* lista);
void list_remove_recurso(t_list *lista, t_recurso *recurso);
void eliminar_archivo(t_pcb *pcb, char* nombre);
t_archivo_abierto* bucsar_archivo_en_pcb(t_pcb* pcb, char* nombre);
int obtener_posicion_recurso(t_list* lista, t_recurso* recurso);
#endif /* PLANIFICACION_CORTO_H_ */
