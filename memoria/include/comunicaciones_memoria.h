#ifndef COMUNICACIONES_MEMORIA_H_
#define COMUNICACIONES_MEMORIA_H_

#include "memoria.h"

typedef struct t_mensajes {
	int cod_op;
	t_handshake origen_mensaje;
	char** parametros;
} t_mensajes;

void manejar_conexion_kernel();
void manejar_conexion_cpu();
void manejar_conexion_fileSystem();
int recibir_conexiones();
void manejar_conexion(int* socket_cliente);

#endif /* COMUNICACIONES_MEMORIA_H_ */

