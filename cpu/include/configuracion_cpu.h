#ifndef CONFIGURACION_CPU_H_
#define CONFIGURACION_CPU_H_

#include <commons/config.h>

typedef struct cpu_config {
    char* IP_MEMORIA;
    char* PUERTO_MEMORIA;
    int RETARDO_INSTRUCCION;
    char* PUERTO_ESCUCHA;
    int TAM_MAX_SEGMENTO;
} t_cpu_config;

extern int socket_kernel;
extern int socket_memoria;
extern t_cpu_config lectura_de_config;

t_cpu_config leer_cpu_config(t_config* config);

#endif /* CONFIGURACION_CPU_H_ */
