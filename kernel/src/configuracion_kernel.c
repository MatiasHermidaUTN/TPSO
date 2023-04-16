#include "../include/configuracion_kernel.h"


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
    lectura_de_config.ESTIMACION_INICIAL          = config_get_int_value(config, "ESTIMACION_INICIAL"); // Hace falta con los int?
    lectura_de_config.HRRN_ALFA                   = config_get_int_value(config, "HRRN_ALFA");
    lectura_de_config.GRADO_MAX_MULTIPROGRAMACION = config_get_int_value(config, "GRADO_MAX_MULTIPROGRAMACION");
    lectura_de_config.RECURSOS                    = strdup(config_get_string_value(config, "RECURSOS"));
    lectura_de_config.INSTANCIAS_RECURSOS         = strdup(config_get_string_value(config, "INSTANCIAS_RECURSOS"));
    // Fijarse si los 3 ints nos conviene que sean ints o directamente char*

    return lectura_de_config;
}


void liberar_estructura_de_config(t_kernel_config config) {
	free(config.IP_MEMORIA);
	free(config.PUERTO_MEMORIA);
	free(config.IP_CPU);
	free(config.PUERTO_CPU);
	free(config.IP_FILESYSTEM);
	free(config.PUERTO_FILESYSTEM);

	free(config.PUERTO_ESCUCHA);
	free(config.ALGORITMO_PLANIFICACION);
	//free(config.ESTIMACION_INICIAL); // Hace falta con los int?
	//free(config.HRRN_ALFA);
	//free(config.GRADO_MAX_MULTIPROGRAMACION);
	free(config.RECURSOS);
	free(config.INSTANCIAS_RECURSOS);
}
