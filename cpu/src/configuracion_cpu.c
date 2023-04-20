#include "../include/configuracion_cpu.h"

int socket_kernel;
t_cpu_config lectura_de_config;

t_cpu_config leer_cpu_config(t_config* config) {
    t_cpu_config lectura_de_config;

    lectura_de_config.IP_MEMORIA     = config_get_string_value(config, "IP_MEMORIA");
    lectura_de_config.PUERTO_MEMORIA = config_get_string_value(config, "PUERTO_MEMORIA");

    lectura_de_config.RETARDO_INSTRUCCION = config_get_string_value(config, "RETARDO_INSTRUCCION");
    lectura_de_config.PUERTO_ESCUCHA      = config_get_string_value(config, "PUERTO_ESCUCHA");
    lectura_de_config.TAM_MAX_SEGMENTO    = config_get_string_value(config, "TAM_MAX_SEGMENTO");

    return lectura_de_config;
}
