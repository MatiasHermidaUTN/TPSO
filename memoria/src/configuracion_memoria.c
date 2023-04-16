#include "../include/configuracion_memoria.h"

t_memoria_config leer_memoria_config(t_config* config) {
    t_memoria_config lectura_de_config;

    lectura_de_config.PUERTO_ESCUCHA       = strdup(config_get_string_value(config, "PUERTO_ESCUCHA"));
	lectura_de_config.TAM_MEMORIA	       = strdup(config_get_string_value(config, "TAM_MEMORIA"));
	lectura_de_config.TAM_SEGMENTO_0	   = strdup(config_get_string_value(config, "TAM_SEGMENTO_0"));
	lectura_de_config.CANT_SEGMENTOS	   = strdup(config_get_string_value(config, "CANT_SEGMENTOS"));
	lectura_de_config.RETARDO_MEMORIA	   = strdup(config_get_string_value(config, "RETARDO_MEMORIA"));
	lectura_de_config.RETARDO_COMPACTACION = strdup(config_get_string_value(config, "RETARDO_COMPACTACION"));
	lectura_de_config.ALGORITMO_ASIGNACION = strdup(config_get_string_value(config, "ALGORITMO_ASIGNACION"));

    return lectura_de_config;
}

void liberar_estructura_de_config(t_memoria_config config) {
	free(config.PUERTO_ESCUCHA);
	free(config.TAM_MEMORIA);
	free(config.TAM_SEGMENTO_0);
	free(config.CANT_SEGMENTOS);
	free(config.RETARDO_MEMORIA);
	free(config.RETARDO_COMPACTACION);
	free(config.ALGORITMO_ASIGNACION);
}
