#ifndef MANEJAR_MENSAJES_H_
#define MANEJAR_MENSAJES_H_

#include "fileSystem.h"
#include "arbir_crear_archivo.h"
#include "truncar_archivo.h"
#include "leer_archivo.h"
#include "escribir_archivo.h"
#include "manejo_punteros.h"

typedef struct args_recibir_mensajes {
	int cod_op;
	char** parametros;
} t_mensajes;

int manejar_mensaje();
void escuchar_kernel();

#endif /* MANEJAR_MENSAJES_H_ */
