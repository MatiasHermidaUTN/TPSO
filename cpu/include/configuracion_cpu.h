#ifndef CONFIGURACION_CPU_H_
#define CONFIGURACION_CPU_H_

#include <commons/config.h>
#include <commons/collections/queue.h>
#include <commons/log.h>
#include <pthread.h>
#include <utils.h>

typedef struct cpu_config {
    char* IP_MEMORIA;
    char* PUERTO_MEMORIA;
    int RETARDO_INSTRUCCION;
    char* PUERTO_ESCUCHA;
    int TAM_MAX_SEGMENTO;
    char* IP_CPU;
} t_cpu_config;

extern int socket_kernel;
extern int socket_memoria;
extern t_cpu_config lectura_de_config;

extern t_log* logger;
extern t_log* my_logger;

t_cpu_config leer_cpu_config(t_config* config);

#endif /* CONFIGURACION_CPU_H_ */
