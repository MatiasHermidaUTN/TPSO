#ifndef CONFIGURACION_KERNEL_H_
#define CONFIGURACION_KERNEL_H_

#include <stdlib.h>
#include <string.h>
#include <commons/config.h>

typedef struct kernel_config {
    char* IP_MEMORIA;
    char* PUERTO_MEMORIA;
    char* IP_CPU;
    char* PUERTO_CPU;
    char* IP_FILESYSTEM;
    char* PUERTO_FILESYSTEM;
    char* PUERTO_ESCUCHA;
    char* ALGORITMO_PLANIFICACION;
    int   ESTIMACION_INICIAL;
    int   HRRN_ALFA;
    int   GRADO_MAX_MULTIPROGRAMACION;
    char* RECURSOS;
    char* INSTANCIAS_RECURSOS;
} t_kernel_config;

t_kernel_config leer_kernel_config(t_config* config);

void liberar_estructura_de_config(t_kernel_config config);

#endif /* CONFIGURACION_KERNEL_H_ */
