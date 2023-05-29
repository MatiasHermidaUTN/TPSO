#ifndef LEER_ARCHIVO_H_
#define LEER_ARCHIVO_H_

#include "fileSystem.h"

//leer
char* leer_archivo(char* nombre_archivo, int apartir_de_donde_leer, int cuanto_leer);
void leer_bloque(char* buffer, uint32_t puntero, int apartir_de_donde_leer_relativo_a_bloque, int* cuanto_leer, int* cantidad_leida);
void leer_indirecto(char* buffer, t_config* archivo_FCB, int bloque_secundario_donde_leer, int* cuanto_leer, int* cantidad_leida);

#endif /* LEER_ARCHIVO_H_ */