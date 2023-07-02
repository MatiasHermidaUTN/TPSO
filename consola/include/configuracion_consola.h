#ifndef CONFIGURACION_CONSOLA_H_
#define CONFIGURACION_CONSOLA_H_

#include <stdlib.h>
#include <commons/config.h>
#include <commons/log.h>
#include <string.h>

typedef struct consola_config {
    char* IP_KERNEL;
    char* PUERTO_KERNEL;
} t_consola_config;

extern int socket_kernel;
extern t_config* config;
extern t_log* logger;

t_consola_config leer_consola_config();
void liberar_estructura_config(t_consola_config config);

#endif /* CONFIGURACION_CONSOLA_H_ */
