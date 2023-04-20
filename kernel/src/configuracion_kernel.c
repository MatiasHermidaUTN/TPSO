#include "../include/configuracion_kernel.h"
#include <commons/log.h>
#include <commons/collections/queue.h>

//SEMAFOROS////////////////////////////////
pthread_mutex_t mutex_contador_pid;
pthread_mutex_t mutex_ready_list; //por si quieren agregar y eliminar de la lista al mismo tiempo.
pthread_mutex_t mutex_new_queue;
sem_t sem_cant_ready;
sem_t sem_cant_new;
sem_t sem_multiprogramacion;
///////////////////////////////////////////

//CONFIG///////////////////////////////////
t_kernel_config lectura_de_config;
///////////////////////////////////////////

//LOGGER///////////////////////////////////
t_log* logger;
///////////////////////////////////////////

t_queue* new_queue;
t_list* ready_list;
t_queue* blocked_queue;

//SOCKETS//////////////////////////////////
int socket_memoria;
int socket_cpu;
int socket_fileSystem;
///////////////////////////////////////////

t_kernel_config leer_kernel_config(t_config* config) {
    t_kernel_config lectura_de_config;

    lectura_de_config.IP_MEMORIA        = strdup(config_get_string_value(config, "IP_MEMORIA"));
    lectura_de_config.PUERTO_MEMORIA    = strdup(config_get_string_value(config, "PUERTO_MEMORIA"));
    lectura_de_config.IP_CPU            = strdup(config_get_string_value(config, "IP_CPU"));
    lectura_de_config.PUERTO_CPU        = strdup(config_get_string_value(config, "PUERTO_CPU"));
    lectura_de_config.IP_FILESYSTEM     = strdup(config_get_string_value(config, "IP_FILESYSTEM"));
    lectura_de_config.PUERTO_FILESYSTEM = strdup(config_get_string_value(config, "PUERTO_FILESYSTEM"));

    lectura_de_config.PUERTO_ESCUCHA              = strdup(config_get_string_value(config, "PUERTO_ESCUCHA"));
    lectura_de_config.ALGORITMO_PLANIFICACION     = strdup(config_get_string_value(config, "ALGORITMO_PLANIFICACION"));
    lectura_de_config.ESTIMACION_INICIAL          = config_get_int_value(config, "ESTIMACION_INICIAL");
    lectura_de_config.HRRN_ALFA                   = config_get_int_value(config, "HRRN_ALFA");
    lectura_de_config.GRADO_MAX_MULTIPROGRAMACION = config_get_int_value(config, "GRADO_MAX_MULTIPROGRAMACION");
    lectura_de_config.RECURSOS                    = strdup(config_get_string_value(config, "RECURSOS"));
    lectura_de_config.INSTANCIAS_RECURSOS         = strdup(config_get_string_value(config, "INSTANCIAS_RECURSOS"));
    // Fijarse si los 3 ints nos conviene que sean ints o directamente char*

    return lectura_de_config;
}

void init_semaforos(){
	pthread_mutex_init(&mutex_contador_pid,NULL);
	pthread_mutex_init(&mutex_ready_list,NULL);
	pthread_mutex_init(&mutex_new_queue,NULL);
	sem_init(&sem_cant_ready,0,0);
	sem_init(&sem_cant_new,0,0);
	sem_init(&sem_multiprogramacion,0, lectura_de_config.GRADO_MAX_MULTIPROGRAMACION);
}

void init_estados(){
	new_queue = queue_create();
	ready_list = list_create();
	blocked_queue = queue_create();
}

void liberar_estructura_config(t_kernel_config config){
	free(config.IP_MEMORIA);
	free(config.PUERTO_MEMORIA);
	free(config.IP_CPU);
	free(config.PUERTO_CPU);
	free(config.IP_FILESYSTEM);
	free(config.PUERTO_FILESYSTEM);
	free(config.PUERTO_ESCUCHA);
	free(config.ALGORITMO_PLANIFICACION);
	free(config.RECURSOS);
	free(config.INSTANCIAS_RECURSOS);

}
