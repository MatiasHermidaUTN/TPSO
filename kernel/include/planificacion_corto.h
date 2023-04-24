#ifndef PLANIFICACION_CORTO_H_
#define PLANIFICACION_CORTO_H_

#include <utils.h>
#include <pthread.h>
#include "configuracion_kernel.h"
#include "comunicaciones_kernel.h"

typedef struct{
	int tiempo;
	t_pcb* pcb;
} t_args_io;

t_registros_cpu init_registros_cpu();
void planificar_corto();
void calcular_prox_rafaga(t_pcb* pcb);
int calcular_R(t_pcb* pcb);
int calcular_tiempo_en_ready(int segundos);
t_pcb* obtener_proximo_a_ejecutar();
int list_remove_element(t_list *self, void *element);
void manejar_io(t_args_io* args_io);

#endif /* PLANIFICACION_CORTO_H_ */
