#ifndef MANMEJO_PUNTEROS_H_
#define MANMEJO_PUNTEROS_H_

#include "fileSystem.h"
#include "configuracion_fileSystem.h"

//manejo de punteros
uint32_t config_get_uint_value(t_config *self, char *key);
uint32_t conseguir_ptr_secundario_para_indirecto(uint32_t puntero_indirecto, int nro_de_ptr_secundario);
void guardar_ptr_secundario_para_indirecto(uint32_t puntero_secundairo, uint32_t puntero_indirecto, int nro_de_ptr_secundario);

//fcb
char* obtener_path_FCB_sin_free(char* nombre_archivo);

#endif /* MANMEJO_PUNTEROS_H_ */
