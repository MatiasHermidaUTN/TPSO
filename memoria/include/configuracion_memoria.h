#ifndef CONFIGURACION_MEMORIA_H_
#define CONFIGURACION_MEMORIA_H_

#include <stdlib.h>
#include <string.h>
#include <commons/config.h>

typedef struct memoria_config {
    char* PUERTO_ESCUCHA;
    char* TAM_MEMORIA;
    char* TAM_SEGMENTO_0;
    char* CANT_SEGMENTOS;
    char* RETARDO_MEMORIA;
    char* RETARDO_COMPACTACION;
    char* ALGORITMO_ASIGNACION;
} t_memoria_config;

t_memoria_config leer_memoria_config(t_config* config);

void liberar_estructura_de_config(t_memoria_config config);

#endif /* CONFIGURACION_MEMORIA_H_ */
