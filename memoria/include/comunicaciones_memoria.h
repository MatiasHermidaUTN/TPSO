#ifndef COMUNICACIONES_MEMORIA_H_
#define COMUNICACIONES_MEMORIA_H_

#include "memoria.h"
#include <commons/log.h>

typedef enum
{
	INICIALIZAR_EL_PROCESO,
	CREAR_SEG,
	ELIMINAR_SEG,
	ELIMINAR_PROCESO,
	ESCRIBIR,
	LEER,
	COMPACTAR,
	ERROR,
} t_instrucciones;

typedef struct t_mensaje {
	t_instrucciones cod_op;
	t_handshake origen_mensaje;
	int pid;
	int id_segmento;
	int tamanio_segmento;
	int dir_fisica;
	int tamanio_buffer;
	char* buffer;
} t_mensajes;

void recibir_parametros(t_mensajes* mensaje);
void manejar_conexion_kernel(int socket_kernel, t_log* logger);
void manejar_conexion_cpu(int socket_cpu, t_log* logger);
void manejar_conexion_fileSystem(int socket_fileSystem, t_log* logger);

#endif /* COMUNICACIONES_MEMORIA_H_ */

