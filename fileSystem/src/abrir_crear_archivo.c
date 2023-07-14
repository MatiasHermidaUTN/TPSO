#include "../include/arbir_crear_archivo.h"

bool existe_archivo(char* nombre_archivo) {
	char* path = obtener_path_FCB_sin_free(nombre_archivo);
	bool retorno = archivo_se_puede_leer(path);
	free(path);
	return retorno;
}

bool archivo_se_puede_leer(char* path)
{
	FILE* f = fopen(path, "r");
	if(f != NULL){
		fclose(f);
		return true;
	} else {
		return false;
	}
}

void crear_archivo(char* nombre_archivo) {
	char* path = obtener_path_FCB_sin_free(nombre_archivo);
	FILE* fcb_nuevo = fopen(path, "w");

	fwrite("NOMBRE_ARCHIVO=", sizeof(char), 15, fcb_nuevo);
	char* aux = strdup(nombre_archivo);
	fwrite(aux, sizeof(char), strlen(aux), fcb_nuevo);
	free(aux);
	fwrite("\n", sizeof(char), 1, fcb_nuevo);
	fwrite("TAMANIO_ARCHIVO=0\n", sizeof(char), 18, fcb_nuevo);
	fwrite("PUNTERO_DIRECTO=\n", sizeof(char), 17, fcb_nuevo);
	fwrite("PUNTERO_INDIRECTO=\n", sizeof(char), 19, fcb_nuevo);
	fclose(fcb_nuevo);
	free(path);
}
