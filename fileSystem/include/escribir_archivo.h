#ifndef ESCRBIR_ARCHIVO_H_
#define ESCRBIR_ARCHIVO_H_

#include "fileSystem.h"

//escribir
void escribir_archivo(char* buffer, char* nombre_archivo, int apartir_de_donde_escribir, int cuanto_escribir);
void sobreescribir_indirecto(char* buffer, t_config* archivo_FCB, int bloque_secundario_donde_escribir, int* cuanto_escribir, int* cantidad_escrita);
void escribir_bloque(char* buffer, uint32_t puntero, int apartir_de_donde_escribir_relativo_a_puntero, int* cuanto_escribir, int* cantidad_escrita);

#endif /* ESCRBIR_ARCHIVO_H_ */