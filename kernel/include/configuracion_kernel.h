#ifndef CONFIGURACION_KERNEL_H_
#define CONFIGURACION_KERNEL_H_

#include <pthread.h>
#include <semaphore.h>
#include <commons/config.h>
#include <commons/collections/queue.h>

#include "planificacion_corto.h"

typedef struct kernel_config{
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


extern pthread_mutex_t mutex_contador_pid;
extern pthread_mutex_t mutex_ready_queue;

extern sem_t sem_ready;


extern t_kernel_config lectura_de_config;
extern t_log* logger;

extern t_queue* new_queue;
extern t_queue* ready_queue;
extern t_queue* blocked_queue;

extern int socket_memoria;
extern int socket_cpu;
extern int socket_fileSystem;


t_kernel_config leer_kernel_config(t_config* config);

void init_semaforos(void);

void init_estados();

void liberar_estructura_config(t_kernel_config config);

#endif /* CONFIGURACION_KERNEL_H_ */
