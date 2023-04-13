#include "../include/configuracion_consola.h"

t_consola_config leer_consola_config(t_config* config) {
    t_consola_config lectura_de_config;

    lectura_de_config.IP_KERNEL     = strdup(config_get_string_value(config, "IP_KERNEL"));
    lectura_de_config.PUERTO_KERNEL = strdup(config_get_string_value(config, "PUERTO_KERNEL"));

    return lectura_de_config;
}

void liberar_estructura_config(t_consola_config config){
	free(config.IP_KERNEL);
	free(config.PUERTO_KERNEL);
}

