#include "../include/configuracion_fileSystem.h"

t_fileSystem_config leer_fileSystem_config(t_config* config) {

	t_fileSystem_config lectura_de_config;

	lectura_de_config.IP_MEMORIA            = config_get_string_value(config, "IP_MEMORIA");
	lectura_de_config.PUERTO_MEMORIA        = config_get_string_value(config, "PUERTO_MEMORIA");
	lectura_de_config.PUERTO_ESCUCHA        = config_get_string_value(config, "PUERTO_ESCUCHA");
	lectura_de_config.PATH_SUPERBLOQUE      = config_get_string_value(config, "PATH_SUPERBLOQUE");
	lectura_de_config.PATH_BITMAP           = config_get_string_value(config, "PATH_BITMAP");
	lectura_de_config.PATH_BLOQUES          = config_get_string_value(config, "PATH_BLOQUES");
	lectura_de_config.PATH_FCB              = config_get_string_value(config, "PATH_FCB");
	lectura_de_config.RETARDO_ACCESO_BLOQUE = config_get_string_value(config, "RETARDO_ACCESO_BLOQUE");

	return lectura_de_config;
}
