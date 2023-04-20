#ifndef CONFIGURACION_CPU_H_
#define CONFIGURACION_CPU_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <commons/config.h>

typedef struct cpu_config {
    char* IP_MEMORIA;
    char* PUERTO_MEMORIA;
    char* RETARDO_INSTRUCCION;
    char* PUERTO_ESCUCHA;
    char* TAM_MAX_SEGMENTO;
} t_cpu_config;

t_cpu_config leer_cpu_config(t_config* config);

void liberar_estructura_config(t_cpu_config lectura_de_config);

#endif /* CONFIGURACION_CPU_H_ */
