#include "../include/escribir_archivo.h"

void escribir_archivo(char* buffer, char* nombre_archivo, int apartir_de_donde_escribir, int cuanto_escribir) {	//llega del switch
	char* path = obtener_path_FCB_sin_free(nombre_archivo);
	t_config* archivo_FCB = iniciar_config(path);
	int tamanio_archivo = config_get_int_value(archivo_FCB, "TAMANIO_ARCHIVO");
	free(path);

	int cantidad_escrita = 0; //para ir recortando el buffer cuando ya leí
	//FAILSAFE
	if (apartir_de_donde_escribir > tamanio_archivo) {		//failsafe - no escribir donde no hay
		log_error(my_logger, "ERROR, apartir_de_donde_leer es > al tamanio_archivo");
		return;
	}
	if(apartir_de_donde_escribir + cuanto_escribir > tamanio_archivo){
		int nuevo_tamanio_archivo = apartir_de_donde_escribir + cuanto_escribir;
		truncar(nombre_archivo, nuevo_tamanio_archivo);
	}

	if (apartir_de_donde_escribir < super_bloque_info.block_size){
		uint32_t puntero_directo = config_get_int_value(archivo_FCB, "PUNTERO_DIRECTO");

		escribir_bloque(buffer, puntero_directo, apartir_de_donde_escribir, &cuanto_escribir, &cantidad_escrita);
		log_info(logger, "Acceso Bloque - Archivo: %s - Bloque Archivo: 0 - Bloque File System: %d", nombre_archivo, puntero_directo); //log obligatorio
		
		sobreescribir_indirecto(buffer, archivo_FCB, PRIMER_BLOQUE_SECUNDARIO, &cuanto_escribir, &cantidad_escrita);
	}
	else {
		int bloque_secundario_inicial = (int) ceil((float) apartir_de_donde_escribir / super_bloque_info.block_size - 1);
		int apartir_de_donde_escribir_relativo_a_bloque = apartir_de_donde_escribir % super_bloque_info.block_size;
		apartir_de_donde_escribir_relativo_a_bloque == 0 ? bloque_secundario_inicial++ : bloque_secundario_inicial;	//pq si apartir es 0, el bloque inicial va a dar el anterior

		uint32_t puntero_indirecto = config_get_int_value(archivo_FCB, "PUNTERO_INDIRECTO");
		uint32_t puntero_secundario = conseguir_ptr_secundario_para_indirecto(puntero_indirecto, bloque_secundario_inicial);

		escribir_bloque(buffer, puntero_secundario, apartir_de_donde_escribir_relativo_a_bloque, &cuanto_escribir, &cantidad_escrita);
		log_info(logger, "Acceso Bloque - Archivo: %s - Bloque Archivo: %d - Bloque File System: %d", nombre_archivo, bloque_secundario_inicial, puntero_secundario); //log obligatorio

		sobreescribir_indirecto(buffer, archivo_FCB, bloque_secundario_inicial + 1, &cuanto_escribir, &cantidad_escrita);
	}
	
	config_destroy(archivo_FCB);
	return;
}

void sobreescribir_indirecto(char* buffer, t_config* archivo_FCB, int bloque_secundario_donde_escribir, int* cuanto_escribir, int* cantidad_escrita) {
	uint32_t puntero_indirecto = config_get_int_value(archivo_FCB, "PUNTERO_INDIRECTO");
	uint32_t puntero_secundario;
	char* nombre_archivo = config_get_string_value(archivo_FCB, "NOMBRE_ARCHIVO");

	while(*cuanto_escribir > 0) {
		puntero_secundario = conseguir_ptr_secundario_para_indirecto(puntero_indirecto, bloque_secundario_donde_escribir);
		escribir_bloque(buffer, puntero_secundario, DESDE_EL_INICIO, cuanto_escribir, cantidad_escrita);
		log_info(logger, "Acceso Bloque - Archivo: %s - Bloque Archivo: %d - Bloque File System: %d", nombre_archivo, bloque_secundario_donde_escribir, puntero_secundario); //log obligatorio
		bloque_secundario_donde_escribir++;
	}
	return;
}

void escribir_bloque(char* buffer, uint32_t puntero, int apartir_de_donde_escribir_relativo_a_bloque, int* cuanto_escribir, int* cantidad_escrita_total) {
	usleep(atoi(lectura_de_config.RETARDO_ACCESO_BLOQUE) * 1000);
	char* buffer_restante = string_substring(buffer, *cantidad_escrita_total, *cuanto_escribir);
	fseek(bloques, puntero * super_bloque_info.block_size + apartir_de_donde_escribir_relativo_a_bloque, SEEK_SET);

	if(apartir_de_donde_escribir_relativo_a_bloque + *cuanto_escribir > super_bloque_info.block_size){
		int dist_fondo_bloque_apartir_de_donde_esc = super_bloque_info.block_size - apartir_de_donde_escribir_relativo_a_bloque;

		fwrite(buffer_restante, dist_fondo_bloque_apartir_de_donde_esc, 1, bloques);
		char a; //TODO: corregir esta villereada
		fread(&a, sizeof(char), 1, bloques);

		*cantidad_escrita_total += dist_fondo_bloque_apartir_de_donde_esc;
		*cuanto_escribir -= dist_fondo_bloque_apartir_de_donde_esc;
	}
	else {
		fwrite(buffer_restante, *cuanto_escribir, 1, bloques);
		char a;
		fread(&a, sizeof(char), 1, bloques);

		*cantidad_escrita_total += *cuanto_escribir;
		*cuanto_escribir = 0;
	}

	//log_warning(my_logger, "buffer_restante: %s\ncuanto_escribir: %d\ncantidad_escrita_total: %d\npuntero: %d", buffer_restante, *cuanto_escribir, *cantidad_escrita_total, puntero);

	free(buffer_restante);
	return;
}
