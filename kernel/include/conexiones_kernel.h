#ifndef CONEXIONES_KERNEL_H_
#define CONEXIONES_KERNEL_H_
#include <pthread.h>
#include <commons/log.h>
#include <utils.h>

typedef struct args_recibir_conexiones{
	int socket_cliente;
	t_log* logger;
}t_args_recibir_conexiones;

int recibir_conexiones(int socket_kernel, t_log* logger);

void manejar_conexion(void* args);


#endif /* CONEXIONES_KERNEL_H_ */
