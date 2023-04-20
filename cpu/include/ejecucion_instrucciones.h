#ifndef EJECUCION_INSTRUCCIONES_H_
#define EJECUCION_INSTRUCCIONES_H_

#include <commons/config.h>
#include <commons/collections/list.h>
#include <commons/string.h>
#include <utils.h>
#include <string.h>
#include "configuracion_cpu.h"

void ejecutar_instrucciones(t_pcb* pcb);
t_enum_instruccion instruccion_a_enum(t_instruccion* instruccion);
void ejecutar_set(t_pcb* pcb, t_instruccion* instruccion_actual);
#endif /* EJECUCION_INSTRUCCIONES_H_ */
