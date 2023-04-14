#ifndef CONEXIONES_KERNEL_H_
#define CONEXIONES_KERNEL_H_
#include <pthread.h>
#include <commons/log.h>
#include <utils.h>
#include <commons/collections/list.h>
#include "configuracion_kernel.h"
#include "comunicaciones_kernel.h"

typedef struct args_recibir_conexiones{
	int socket_cliente;
	t_log* logger;
}t_args_recibir_conexiones;

int recibir_conexiones(int socket_kernel, t_log* logger);

void manejar_conexion(void* args);

void init_conexiones(t_kernel_config lectura_de_config, t_log* logger, int* socket_memoria, int* socket_cpu, int* socket_fileSystem);


#endif /* CONEXIONES_KERNEL_H_ */
