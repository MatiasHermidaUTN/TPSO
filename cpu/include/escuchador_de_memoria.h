#ifndef ESCUHADOR_DE_MEMORIA_H
#define ESCUHADOR_DE_MEMORIA_H

#include <utils.h>
#include <pthread.h>
#include "configuracion_cpu.h"
#include "ejecucion_instrucciones.h"

typedef struct {
	t_pcb* pcb;
	int numero_segmento;
	int direccion_fisica;
	char* nombre_registro;
} t_args_log_acceso_memoria;

void log_acceso_memoria();

t_args_log_acceso_memoria* list_remove_solicitud(int pid);

#endif /* ESCUHADOR_DE_MEMORIA_H */
