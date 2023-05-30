#ifndef COMUNICACIONES_MEMORIA_H_
#define COMUNICACIONES_MEMORIA_H_

#include "memoria.h"
#include "conexiones_memoria.h"
#include <commons/log.h>

typedef struct t_mensaje {
	int cod_op;
	t_handshake origen_mensaje;
	char** parametros;
} t_mensaje;

void manejar_conexion_kernel(t_log* logger);
void manejar_conexion_cpu(t_log* logger);
void manejar_conexion_fileSystem(t_log* logger);

#endif /* COMUNICACIONES_MEMORIA_H_ */

