#ifndef ESCUHADOR_DE_MEMORIA_H
#define ESCUHADOR_DE_MEMORIA_H

#include <utils.h>
#include <pthread.h>
#include "configuracion_cpu.h"

typedef struct {
	int pid;
	int numero_segmento;
	int direccion_fisica;
} t_args_log_acceso_memoria;

void log_acceso_memoria();

#endif /* ESCUHADOR_DE_MEMORIA_H */
