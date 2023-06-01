#include "../include/configuracion_cpu.h"

int socket_kernel;
int socket_memoria;
t_cpu_config lectura_de_config;

t_log* logger;

t_queue* queue_solicitudes_acceso_memoria;

pthread_mutex_t* mutex_queue_solicitudes_acceso_memoria; //TODO: fijarse si necesariamente no tiene que ser un puntero o está bien así

t_cpu_config leer_cpu_config(t_config* config) {
    t_cpu_config lectura_de_config;

    lectura_de_config.IP_MEMORIA     = config_get_string_value(config, "IP_MEMORIA");
    lectura_de_config.PUERTO_MEMORIA = config_get_string_value(config, "PUERTO_MEMORIA");

    lectura_de_config.RETARDO_INSTRUCCION = config_get_int_value(config, "RETARDO_INSTRUCCION");
    lectura_de_config.PUERTO_ESCUCHA      = config_get_string_value(config, "PUERTO_ESCUCHA");
    lectura_de_config.TAM_MAX_SEGMENTO    = config_get_int_value(config, "TAM_MAX_SEGMENTO");

    return lectura_de_config;
}
