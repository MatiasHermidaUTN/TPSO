#ifndef CONFIGURACION_KERNEL_H_
#define CONFIGURACION_KERNEL_H_

#include <utils.h>
#include <commons/config.h>
#include <pthread.h>
#include <commons/collections/queue.h>
#include "planificacion_largo.h"
#include <semaphore.h>

typedef struct kernel_config {
    char* IP_MEMORIA;
    char* PUERTO_MEMORIA;
    char* IP_CPU;
    char* PUERTO_CPU;
    char* IP_FILESYSTEM;
    char* PUERTO_FILESYSTEM;
    char* PUERTO_ESCUCHA;
    char* ALGORITMO_PLANIFICACION;
    double   ESTIMACION_INICIAL;
    double   HRRN_ALFA;
    int   GRADO_MAX_MULTIPROGRAMACION;
    char** RECURSOS;
    char** INSTANCIAS_RECURSOS;
} t_kernel_config;

typedef struct {
	char* nombre;
	int cantidad_disponibles;
	t_queue* cola_bloqueados;
	pthread_mutex_t mutex_archivo;
} t_recurso;

extern pthread_mutex_t mutex_contador_pid;
extern pthread_mutex_t mutex_new_queue;
extern pthread_mutex_t mutex_ready_list;
extern pthread_mutex_t mutex_cantidad_de_reads_writes;
extern pthread_mutex_t mutex_msj_memoria;
extern pthread_mutex_t mutex_pcbs_en_io;


extern sem_t sem_cant_ready;
extern sem_t sem_cant_new;
extern sem_t sem_multiprogramacion;
extern sem_t sem_respuesta_fs;
extern sem_t sem_compactacion;


extern t_kernel_config lectura_de_config;
extern t_log* logger;

extern t_queue* new_queue;
extern t_list* ready_list;

extern int socket_memoria;
extern int socket_cpu;
extern int socket_fileSystem;

extern t_list* list_recursos;
extern t_list* list_archivos;

extern t_list* pcbs_en_io;


extern t_pcb* proximo_pcb_a_ejecutar_forzado;
extern t_msj_kernel_fileSystem respuesta_fs_global;
extern int cantidad_de_reads_writes;


t_kernel_config leer_kernel_config(t_config* config);
void init_semaforos(void);
void init_estados();

void liberar_estructura_config(t_kernel_config config);

void log_pids();
char* obtener_pids(t_list* lista_pcbs);

void mantener_pcb_en_exec(t_pcb* pcb_recibido);
void ready_list_push(t_pcb* pcb_recibido);
void calcular_prox_rafaga(t_pcb* pcb);

#endif /* CONFIGURACION_KERNEL_H_ */
