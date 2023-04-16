#ifndef CONEXIONES_KERNEL_H_
#define CONEXIONES_KERNEL_H_

#include <pthread.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>

#include <utils.h>
#include "configuracion_kernel.h"
#include "comunicaciones_kernel.h"

typedef struct args_recibir_conexiones {
	int socket_cliente;
	t_log* logger;
	int config_estimado_de_proxima_rafaga;
	t_queue* new;
} t_args_recibir_conexiones;

int recibir_conexiones(int socket_kernel, t_log* logger, int config_estimacion_inicial);

void manejar_conexion(void* args);

void inicializar_conexiones(t_kernel_config lectura_de_config, t_log* logger, int* socket_memoria, int* socket_cpu, int* socket_fileSystem);

#endif /* CONEXIONES_KERNEL_H_ */
