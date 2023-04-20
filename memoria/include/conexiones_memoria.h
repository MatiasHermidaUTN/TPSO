#ifndef CONEXIONES_MEMORIA_H_
#define CONEXIONES_MEMORIA_H_

#include <pthread.h>
#include <commons/log.h>
#include <utils.h>
#include "comunicaciones_memoria.h"

int recibir_conexiones(int socket_memoria, t_log* logger);

void manejar_conexion(void* args);


#endif /* CONEXIONES_MEMORIA_H_ */
