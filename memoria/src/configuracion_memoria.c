#include "../include/configuracion_memoria.h"

t_memoria_config leer_memoria_config(t_config* config) {
    t_memoria_config lectura_de_config;

    lectura_de_config.PUERTO_ESCUCHA       = config_get_string_value(config, "PUERTO_ESCUCHA");
	lectura_de_config.TAM_MEMORIA	       = config_get_string_value(config, "TAM_MEMORIA");
	lectura_de_config.TAM_SEGMENTO_0	   = config_get_string_value(config, "TAM_SEGMENTO_0");
	lectura_de_config.CANT_SEGMENTOS	   = config_get_string_value(config, "CANT_SEGMENTOS");
	lectura_de_config.RETARDO_MEMORIA	   = config_get_string_value(config, "RETARDO_MEMORIA");
	lectura_de_config.RETARDO_COMPACTACION = config_get_string_value(config, "RETARDO_COMPACTACION");
	lectura_de_config.ALGORITMO_ASIGNACION = config_get_string_value(config, "ALGORITMO_ASIGNACION");

    return lectura_de_config;
}
