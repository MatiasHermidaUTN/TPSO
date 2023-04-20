#ifndef PLANIFICACION_CORTO_H_
#define PLANIFICACION_CORTO_H_

#include <pthread.h>

#include <utils.h>
#include "configuracion_kernel.h"
#include "comunicaciones_kernel.h"

t_registros_cpu init_registros_cpu();

t_pcb* crear_pcb(t_list* instrucciones, int socket_consola);

void planificar_corto(void* args);

t_pcb *queue_pop_con_mutex(t_queue* queue, pthread_mutex_t* mutex);

#endif /* PLANIFICACION_CORTO_H_ */
