#include "../include/configuracion_consola.h"

t_consola_config leer_consola_config(t_config* config) {
    t_consola_config lectura_de_config;

    lectura_de_config.IP_KERNEL     = config_get_string_value(config, "IP_KERNEL");
    lectura_de_config.PUERTO_KERNEL = config_get_string_value(config, "PUERTO_KERNEL");

    return lectura_de_config;
}

