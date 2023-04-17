#ifndef PLANIFICACION_CORTO_H_
#define PLANIFICACION_CORTO_H_

#include <utils.h>
#include <pthread.h>
#include "configuracion_kernel.h"

typedef struct registros_cpu{
	char AX,BX,CX,DX [4];
	char EAX,EBX,ECX,EDX [8];
	char RAX,RBX,RCX,RDX [16];
}t_registros_cpu;

typedef struct pcb{
	int pid;
	t_list* instrucciones;
	int pc;
	t_registros_cpu registros_cpu;
	t_list* tabla_segmentos;
	int estimado_prox_rafaga;
	int estimado_llegada_ready;
	t_list* archivos_abiertos;
	int socket_consola; //para mandarle mensaje que cuando termina

}t_pcb;

t_registros_cpu init_registros_cpu();
t_pcb* crear_pcb(t_list* instrucciones,int socket_consola);
void planificar_corto();
t_pcb *queue_pop_con_mutex(t_queue* queue, pthread_mutex_t mutex);

#endif /* PLANIFICACION_CORTO_H_ */
