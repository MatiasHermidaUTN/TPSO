#ifndef ABRIR_CREAR_ARCHIVO_H_
#define ABRIR_CREAR_ARCHIVO_H_

#include "fileSystem.h"

//abrir
bool existe_archivo(char* nombre_archivo);
bool archivo_se_puede_leer(char* path);
//crear
void crear_archivo(char* nombre_archivo);

#endif /* ABRIR_CREAR_ARCHIVO_H_ */