#include "../include/configuracion_cpu.h"

t_cpu_config leer_cpu_config(t_config* config) {
    t_cpu_config lectura_de_config;

    lectura_de_config.IP_MEMORIA     = strdup(config_get_string_value(config, "IP_MEMORIA"));
    lectura_de_config.PUERTO_MEMORIA = strdup(config_get_string_value(config, "PUERTO_MEMORIA"));

    lectura_de_config.RETARDO_INSTRUCCION = strdup(config_get_string_value(config, "RETARDO_INSTRUCCION"));
    lectura_de_config.PUERTO_ESCUCHA      = strdup(config_get_string_value(config, "PUERTO_ESCUCHA"));
    lectura_de_config.TAM_MAX_SEGMENTO    = strdup(config_get_string_value(config, "TAM_MAX_SEGMENTO"));

    return lectura_de_config;
}

void liberar_estructura_de_config(t_cpu_config config) {
	free(config.IP_MEMORIA);
	free(config.PUERTO_MEMORIA);

	free(config.RETARDO_INSTRUCCION);
	free(config.PUERTO_ESCUCHA);
	free(config.TAM_MAX_SEGMENTO);
}
