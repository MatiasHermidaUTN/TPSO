#ifndef EJECUCION_INSTRUCCIONES_H_
#define EJECUCION_INSTRUCCIONES_H_

#include <commons/config.h>
#include <commons/collections/list.h>
#include <commons/string.h>
#include <utils.h>
#include <string.h>
#include "configuracion_cpu.h"

extern t_log* logger;

void ejecutar_instrucciones(t_pcb* pcb);

t_enum_instruccion instruccion_a_enum(t_instruccion* instruccion);

void ejecutar_set(t_pcb* pcb, t_instruccion* instruccion_actual);

void enviar_pcb_a_kernel(t_pcb* pcb, t_msj_kernel_cpu mensaje, char** parametros, t_list* list_parametros, int cantidad_de_parametros);

#endif /* EJECUCION_INSTRUCCIONES_H_ */
