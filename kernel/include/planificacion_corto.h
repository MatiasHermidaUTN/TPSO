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

int calcular_R(t_pcb* pcb);
int calcular_tiempo_en_ready(int segundos);

t_pcb* obtener_proximo_a_ejecutar();

void manejar_io(t_args_io* args_io);

void wait_recurso(t_pcb* pcb, char* recurso);
t_recurso* buscar_recurso(char* nombre_recurso, t_list* lista);
void signal_recurso(t_pcb* pcb, char* recurso, int esta_en_exit);
void eliminar_recurso_de_lista(t_list* recursos, char* nombre_recurso, int esta_en_exit);
void exit_proceso(t_pcb* pcb, t_msj_kernel_consola mensaje);
void signal_de_todos_los_recursos(t_pcb* pcb);
char* mensaje_de_finalizacion_a_string(t_msj_kernel_consola mensaje);

void list_remove_pcb(t_list* lista, t_pcb* pcb);
t_pcb* list_get_max_R(t_list* lista);

void list_remove_recurso(t_list* lista, t_recurso* recurso);

void cerrar_archivo(t_pcb* pcb_recibido, char* nombre_archivo);
t_archivo_abierto* eliminar_archivo(t_pcb* pcb, char* nombre);
void cerrar_todos_los_archivos(t_pcb* pcb);
t_archivo_abierto* buscar_archivo_en_pcb(t_pcb* pcb, char* nombre);
void bloquear_pcb_por_archivo(t_pcb* pcb, char* nombre_archivo);

void crear_segmento(t_pcb* pcb_recibido, char** parametros);
void actualizar_segmentos(t_pcb* pcb_en_exec, t_list* procesos);
void actualizar_segmentos_de_lista(t_list* lista, t_list* procesos);
void actualizar_segmentos_de_cola(t_queue* cola, t_list* procesos);
void actualizar_segmentos_de_pcb(t_pcb* pcb, t_list* segmentos);

t_proceso_actualizado* list_remove_if_pid_equals_to(t_list* procesos, int pid);

#endif /* PLANIFICACION_CORTO_H_ */
