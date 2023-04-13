#ifndef CONFIGURACION_CONSOLA_H_
#define CONFIGURACION_CONSOLA_H_

#include <stdlib.h>
#include <commons/config.h>
#include <string.h>

typedef struct consola_config {
    char* IP_KERNEL;
    char* PUERTO_KERNEL;
} t_consola_config;

t_consola_config leer_consola_config(t_config* config);
void liberar_estructura_config(t_consola_config config);

#endif /* CONFIGURACION_CONSOLA_H_ */
