#ifndef PLANIFICACION_CORTO_H_
#define PLANIFICACION_CORTO_H_

#include <utils.h>
#include <pthread.h>
#include "configuracion_kernel.h"
#include "comunicaciones_kernel.h"

t_registros_cpu init_registros_cpu();
t_pcb* crear_pcb(t_list* instrucciones,int socket_consola);
void planificar_corto();

#endif /* PLANIFICACION_CORTO_H_ */
