#include "manejo_punteros.h"

uint32_t config_get_uint_value(t_config *self, char *key) {
	char *value = config_get_string_value(self, key);
	char *ptr;
	return (uint32_t) strtoul(value, &ptr, 10);
}

uint32_t conseguir_ptr_secundario_para_indirecto(uint32_t puntero_indirecto, int nro_de_ptr_secundario){
	usleep(atoi(lectura_de_config.RETARDO_ACCESO_BLOQUE) * 1000);
	uint32_t puntero;
	fseek(bloques, puntero_indirecto * super_bloque_info.block_size + sizeof(puntero) * (nro_de_ptr_secundario-1), SEEK_SET);
	fread(&puntero, sizeof(puntero), 1, bloques);
	return puntero;
}

void guardar_ptr_secundario_para_indirecto(uint32_t puntero_secundairo, uint32_t puntero_indirecto, int nro_de_ptr_secundario){
	usleep(atoi(lectura_de_config.RETARDO_ACCESO_BLOQUE) * 1000);
	fseek(bloques, puntero_indirecto * super_bloque_info.block_size + sizeof(puntero_secundairo) * nro_de_ptr_secundario, SEEK_SET);
	fwrite(&puntero_secundairo, sizeof(puntero_secundairo), 1, bloques);				//anoto nuevo puntero_secundario en bloque de puntero_indirecto
}

char* obtener_path_FCB_sin_free(char* nombre_archivo){
	char* path = malloc(strlen(lectura_de_config.PATH_FCB) + strlen(nombre_archivo) + 1);
	strcpy(path, lectura_de_config.PATH_FCB);
	strcat(path, nombre_archivo);		//se asume que en la FS.config el path_fcb tiene / al final quedando: "/home/utnso/fs/fcb/"
	return path;
}