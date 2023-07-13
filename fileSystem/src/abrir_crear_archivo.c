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
	//t_config* FCBdefault = iniciar_config("./fileSystem/FCBdefault");
	t_config* FCBdefault = iniciar_config("../FCBdefault"); //TODO por que antes andaba?
	config_set_value(FCBdefault, "NOMBRE_ARCHIVO", nombre_archivo);
	config_set_value(FCBdefault, "TAMANIO_ARCHIVO", "0");
	config_set_value(FCBdefault, "PUNTERO_DIRECTO", "");
	config_set_value(FCBdefault, "PUNTERO_INDIRECTO", "");
	config_save_in_file(FCBdefault, path);
	config_destroy(FCBdefault);					//cierro el FCB
	free(path);
}
