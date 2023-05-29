#ifndef CONFIGURACION_FILESYSTEM_H_
#define CONFIGURACION_FILESYSTEM_H_

#include <commons/config.h>
#include "fileSystem.h"

typedef struct fileSystem_config {
	char* IP_MEMORIA;
	char* PUERTO_MEMORIA;
	char* PUERTO_ESCUCHA;
	char* PATH_SUPERBLOQUE;
	char* PATH_BITMAP;
	char* PATH_BLOQUES;
	char* PATH_FCB;
	char* RETARDO_ACCESO_BLOQUE;
} t_fileSystem_config;

t_fileSystem_config leer_fileSystem_config(t_config* config);

void init_superbloque();
void init_bitmap();
void init_bloques();
void init_carpeta_fcbs();

#endif /* CONFIGURACION_FILESYSTEM_H_ */
