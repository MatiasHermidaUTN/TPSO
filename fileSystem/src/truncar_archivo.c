#include "truncar_archivo.h"

void truncar(char* nombre_archivo, int nuevo_tamanio_archivo){
	char* path = obtener_path_FCB_sin_free(nombre_archivo);
	t_config* archivo_FCB = iniciar_config(path);

	int tamanio_archivo = config_get_int_value(archivo_FCB, "TAMANIO_ARCHIVO");

	if (nuevo_tamanio_archivo > tamanio_archivo){
		agrandas_archivo(archivo_FCB, nombre_archivo, nuevo_tamanio_archivo);
	}
	else if (nuevo_tamanio_archivo < tamanio_archivo) {
		achicas_archivo(archivo_FCB, nombre_archivo, nuevo_tamanio_archivo);
	}

	char* nuevo_tamanio_string = string_itoa(nuevo_tamanio_archivo);
	config_set_value(archivo_FCB, "TAMANIO_ARCHIVO", nuevo_tamanio_string);
	free(nuevo_tamanio_string); //itoa creo que hace malloc y si lo pones directo en la funcion no lo podes liberar despues
	config_save_in_file(archivo_FCB, path);
	config_destroy(archivo_FCB);
	free(path);
	printf("\n");
	printf("unos dsp truncar: %d\n", cant_unos_en_bitmap());
	return;
}

void achicas_archivo(t_config* archivo_FCB, char* nombre_archivo, int nuevo_tamanio_archivo){
	int tamanio_archivo = config_get_int_value(archivo_FCB, "TAMANIO_ARCHIVO");

	//entro si tiene puntero_directo y ya tiene puntero_indirecto
	if(tamanio_archivo > super_bloque_info.block_size){
		uint32_t puntero_indirecto = config_get_uint_value(archivo_FCB, "PUNTERO_INDIRECTO");

		int cant_punteros_secundarios = (int) ceil((float) tamanio_archivo / super_bloque_info.block_size - 1);
		int cant_bloques_secundarios_necesarios = (int) ceil((float) (nuevo_tamanio_archivo ? nuevo_tamanio_archivo : 1 ) / super_bloque_info.block_size - 1);
		if(cant_punteros_secundarios > cant_bloques_secundarios_necesarios){
			for( ; cant_punteros_secundarios > cant_bloques_secundarios_necesarios ; cant_punteros_secundarios--) {
				uint32_t puntero = conseguir_ptr_secundario_para_indirecto(puntero_indirecto, cant_punteros_secundarios);
				liberar_bloque(puntero);
			}
		}
		if(nuevo_tamanio_archivo <= super_bloque_info.block_size){
			liberar_bloque(puntero_indirecto);
			config_set_value(archivo_FCB, "PUNTERO_INDIRECTO", "");
		}
	}
	if(nuevo_tamanio_archivo == 0){
		uint32_t puntero_directo = config_get_uint_value(archivo_FCB, "PUNTERO_DIRECTO");
		liberar_bloque(puntero_directo);
		config_set_value(archivo_FCB, "PUNTERO_DIRECTO", "");
	}
	return;
}

void agrandas_archivo(t_config* archivo_FCB, char* nombre_archivo, int nuevo_tamanio_archivo){
	int tamanio_archivo = config_get_int_value(archivo_FCB, "TAMANIO_ARCHIVO");

	//CADENA DE IFs, va entrando si cumple condicion
	//entro si no tiene puntero_directo
	if(tamanio_archivo == 0){
		uint32_t puntero_directo = dame_un_bloque_libre();
		config_set_value(archivo_FCB, "PUNTERO_DIRECTO", string_itoa(puntero_directo));
	}
	//entro si tiene puntero_directo, no tiene puntero_indirecto y si el nuevo tamanio requiere de punteros_secundarios
	if(tamanio_archivo <= super_bloque_info.block_size && nuevo_tamanio_archivo > super_bloque_info.block_size){
		uint32_t puntero_indirecto = dame_un_bloque_libre();
		config_set_value(archivo_FCB, "PUNTERO_INDIRECTO", string_itoa(puntero_indirecto));

		int cant_punteros_secundarios = 0;
		int cant_bloques_secundarios_necesarios = (int) ceil((float) nuevo_tamanio_archivo / super_bloque_info.block_size - 1);
		for( ; cant_punteros_secundarios < cant_bloques_secundarios_necesarios ; cant_punteros_secundarios++) {
			uint32_t puntero = dame_un_bloque_libre();
			guardar_ptr_secundario_para_indirecto(puntero, puntero_indirecto, cant_punteros_secundarios);
		}
	}
	//entro si tiene puntero_directo, tiene puntero_indirecto (tiene 1 o + punteros_secundarios)
	if(tamanio_archivo > super_bloque_info.block_size){
		uint32_t puntero_indirecto = config_get_uint_value(archivo_FCB, "PUNTERO_INDIRECTO");

		int cant_punteros_secundarios = (int) ceil((float) tamanio_archivo / super_bloque_info.block_size - 1);
		int cant_bloques_secundarios_necesarios = (int) ceil((float) nuevo_tamanio_archivo / super_bloque_info.block_size - 1);
		for( ; cant_punteros_secundarios < cant_bloques_secundarios_necesarios ; cant_punteros_secundarios++) {
			uint32_t puntero = dame_un_bloque_libre();
			guardar_ptr_secundario_para_indirecto(puntero, puntero_indirecto, cant_punteros_secundarios);
		}
	}
	return;
}

uint32_t dame_un_bloque_libre() {
	for (int i = 0; i < bitarray_get_max_bit(bitarray_de_bitmap); i++) {
		if (bitarray_test_bit(bitarray_de_bitmap, i) == 0) {
			bitarray_set_bit(bitarray_de_bitmap, i);
			msync(bitmap_pointer, tamanioBitmap, MS_SYNC);
			return i;
		}
	}
	log_error(logger, "No existe mas espacio en el disco");
	return -1;
}

void liberar_bloque(uint32_t puntero){
	bitarray_clean_bit(bitarray_de_bitmap, puntero);
	msync(bitmap_pointer, tamanioBitmap, MS_SYNC);
}