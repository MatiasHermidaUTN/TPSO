#ifndef COMUNICACIONES_MEMORIA_H_
#define COMUNICACIONES_MEMORIA_H_

#include <commons/log.h>

void manejar_conexion_kernel(int socket_kernel, t_log* logger);
void manejar_conexion_cpu(int socket_cpu, t_log* logger);
void manejar_conexion_fileSystem(int socket_fileSystem, t_log* logger);

#endif /* COMUNICACIONES_MEMORIA_H_ */
