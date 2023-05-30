#ifndef TRUNCAR_ARCHIVO_H_
#define TRUNCAR_ARCHIVO_H_

#include "fileSystem.h"
#include "configuracion_fileSystem.h"

//truncar
void truncar(char* nombre_archivo, int nuevo_tamanio_archivo);
void achicas_archivo(t_config* archivo_FCB, char* nombre_archivo, int nuevo_tamanio_archivo);
void agrandas_archivo(t_config* archivo_FCB, char* nombre_archivo, int nuevo_tamanio_archivo);

//bitmap
uint32_t dame_un_bloque_libre();
void liberar_bloque(uint32_t puntero);

#endif /* TRUNCAR_ARCHIVO_H_ */
