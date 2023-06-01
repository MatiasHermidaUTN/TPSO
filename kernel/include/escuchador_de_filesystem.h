#ifndef ESCUHADOR_DE_FILESYSTEM_H
#define ESCUHADOR_DE_FILESYSTEM_H

#include <utils.h>
#include <pthread.h>
#include "configuracion_kernel.h"
#include "planificacion_corto.h"

void escuchar_de_filesystem();
void desbloquear_pcb_por_archivo(char* nombre_archivo,int pid);
t_pcb* obtener_pcb_de_cola(t_recurso* archivo, int pid);

#endif /* ESCUHADOR_DE_FILESYSTEM_H */
