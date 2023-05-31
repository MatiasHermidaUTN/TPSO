#include "../include/configuracion_memoria.h"

//-----VARIABLES COMPARTIDAS-----
t_memoria_config lectura_de_config;

int socket_memoria;
int socket_kernel;
int socket_cpu;
int socket_fileSystem;

pthread_mutex_t mutex_cola_msj;
sem_t sem_cant_msj;
t_list* lista_fifo_msj;

t_log* logger;
//-------------------------------

void leer_memoria_config(t_config* config) {
    lectura_de_config.PUERTO_ESCUCHA       = config_get_string_value(config, "PUERTO_ESCUCHA");
	lectura_de_config.TAM_MEMORIA	       = config_get_string_value(config, "TAM_MEMORIA");
	lectura_de_config.TAM_SEGMENTO_0	   = config_get_string_value(config, "TAM_SEGMENTO_0");
	lectura_de_config.CANT_SEGMENTOS	   = config_get_string_value(config, "CANT_SEGMENTOS");
	lectura_de_config.RETARDO_MEMORIA	   = config_get_string_value(config, "RETARDO_MEMORIA");
	lectura_de_config.RETARDO_COMPACTACION = config_get_string_value(config, "RETARDO_COMPACTACION");
	lectura_de_config.ALGORITMO_ASIGNACION = config_get_string_value(config, "ALGORITMO_ASIGNACION");

    return;
}

void liberar_estructura_config() {
	free(lectura_de_config.PUERTO_ESCUCHA);
	free(lectura_de_config.TAM_MEMORIA);
	free(lectura_de_config.TAM_SEGMENTO_0);
	free(lectura_de_config.CANT_SEGMENTOS);
	free(lectura_de_config.RETARDO_MEMORIA);
	free(lectura_de_config.RETARDO_COMPACTACION);
	free(lectura_de_config.ALGORITMO_ASIGNACION);
}