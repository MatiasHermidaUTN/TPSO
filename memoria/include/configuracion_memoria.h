#ifndef CONFIGURACION_MEMORIA_H_
#define CONFIGURACION_MEMORIA_H_

#include <commons/config.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include <pthread.h>
#include <semaphore.h>
#include <utils.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct memoria_config {
    char* PUERTO_ESCUCHA;
    char* TAM_MEMORIA;
    char* TAM_SEGMENTO_0;
    char* CANT_SEGMENTOS;
    char* RETARDO_MEMORIA;
    char* RETARDO_COMPACTACION;
    char* ALGORITMO_ASIGNACION;
    char* IP_MEMORIA;
} t_memoria_config;

extern t_memoria_config lectura_de_config;

extern int socket_memoria;
extern int socket_kernel;
extern int socket_cpu;
extern int socket_fileSystem;

extern pthread_mutex_t mutex_cola_msj;
extern sem_t sem_cant_msj;
extern t_list* lista_fifo_msj;

extern t_log* logger;
extern t_log* my_logger;

void leer_memoria_config(t_config* config);
void liberar_estructura_config();

#endif /* CONFIGURACION_MEMORIA_H_ */
