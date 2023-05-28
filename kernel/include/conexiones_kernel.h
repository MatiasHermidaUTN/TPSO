#ifndef CONEXIONES_KERNEL_H_
#define CONEXIONES_KERNEL_H_
#include <commons/log.h>
#include <utils.h>
#include <pthread.h>

#include <commons/collections/list.h>
#include "configuracion_kernel.h"
#include "comunicaciones_kernel.h"
#include "planificacion_corto.h"

typedef struct args_recibir_conexiones {
	int socket_cliente;
} t_args_recibir_conexiones;

extern t_pcb* pcb_running;

extern t_queue* new_queue;
extern t_queue* ready_queue;
extern t_queue* blocked_queue;

int recibir_conexiones(int socket_kernel);
void manejar_conexion(void* args);

t_pcb* crear_pcb(t_list* instrucciones,int socket_consola);

t_registros_cpu init_registros_cpu();

void init_conexiones();

#endif /* CONEXIONES_KERNEL_H_ */
