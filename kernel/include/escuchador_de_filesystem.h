#ifndef ESCUHADOR_DE_FILESYSTEM_H
#define ESCUHADOR_DE_FILESYSTEM_H

#include "configuracion_kernel.h"
#include "planificacion_corto.h"
#include <utils.h>

void escuchar_de_filesystem();
t_pcb* obtener_pcb_de_cola(t_recurso* archivo, int pid);

#endif /* ESCUHADOR_DE_FILESYSTEM_H */
