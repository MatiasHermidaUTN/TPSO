#include "escribir_archivo.h"

void escribir_archivo(char* buffer, char* nombre_archivo, int apartir_de_donde_escribir, int cuanto_escribir) {	//llega del switch
	char* path = obtener_path_FCB_sin_free(nombre_archivo);
	t_config* archivo_FCB = iniciar_config(path);
	int tamanio_archivo = config_get_int_value(archivo_FCB, "TAMANIO_ARCHIVO");
	free(path);

	int cantidad_escrita = 0; //para ir recortando el buffer cuando ya leí
	//FAILSAFE
	if (apartir_de_donde_escribir > tamanio_archivo) {		//failsafe - no escribir donde no hay
		log_error(logger, "ERROR, apartir_de_donde_leer es > al tamanio_archivo");
		return;
	}
	if(apartir_de_donde_escribir + cuanto_escribir > tamanio_archivo){
		int nuevo_tamanio_archivo = apartir_de_donde_escribir + cuanto_escribir;
		truncar(nombre_archivo, nuevo_tamanio_archivo);
		printf("unos dsp truncar: %d\n", cant_unos_en_bitmap());
	}

	if (apartir_de_donde_escribir < super_bloque_info.block_size){
		uint32_t puntero_directo = config_get_int_value(archivo_FCB, "PUNTERO_DIRECTO");
		fseek(bloques, puntero_directo * super_bloque_info.block_size, SEEK_SET);

		escribir_bloque(buffer, puntero_directo, apartir_de_donde_escribir, &cuanto_escribir, &cantidad_escrita);
		sobreescribir_indirecto(buffer, archivo_FCB, PRIMER_BLOQUE_SECUNDARIO, &cuanto_escribir, &cantidad_escrita);
	}
	else {
		int bloque_secundario_inicial = (int) ceil((float) apartir_de_donde_escribir / super_bloque_info.block_size - 1);
		int apartir_de_donde_escribir_relativo_a_bloque = apartir_de_donde_escribir % super_bloque_info.block_size;
		apartir_de_donde_escribir_relativo_a_bloque==0? bloque_secundario_inicial++ : bloque_secundario_inicial;	//pq si apartir es 0, el bloque inicial va a dar el anterior

		uint32_t puntero_indirecto = config_get_int_value(archivo_FCB, "PUNTERO_INDIRECTO");
		uint32_t puntero_secundario = conseguir_ptr_secundario_para_indirecto(puntero_indirecto, bloque_secundario_inicial);

		escribir_bloque(buffer, puntero_secundario, apartir_de_donde_escribir_relativo_a_bloque, &cuanto_escribir, &cantidad_escrita);
		sobreescribir_indirecto(buffer, archivo_FCB, bloque_secundario_inicial + 1, &cuanto_escribir, &cantidad_escrita);
	}
	
	config_destroy(archivo_FCB);
	return;
}

void sobreescribir_indirecto(char* buffer, t_config* archivo_FCB, int bloque_secundario_donde_escribir, int* cuanto_escribir, int* cantidad_escrita) {
	uint32_t puntero_indirecto = config_get_int_value(archivo_FCB, "PUNTERO_INDIRECTO");
	uint32_t puntero_secundario;			

	while(*cuanto_escribir > 0) {
		puntero_secundario = conseguir_ptr_secundario_para_indirecto(puntero_indirecto, bloque_secundario_donde_escribir);
		escribir_bloque(buffer, puntero_secundario, DESDE_EL_INICIO, cuanto_escribir, cantidad_escrita);
		bloque_secundario_donde_escribir++;
	}
	return;
}

void escribir_bloque(char* buffer, uint32_t puntero, int apartir_de_donde_escribir_relativo_a_bloque, int* cuanto_escribir, int* cantidad_escrita) {
	usleep(atoi(lectura_de_config.RETARDO_ACCESO_BLOQUE) * 1000);
	char* copia_buffer = (char*) string_substring(buffer, *cantidad_escrita, *cuanto_escribir);
	fseek(bloques, puntero * super_bloque_info.block_size + apartir_de_donde_escribir_relativo_a_bloque, SEEK_SET);

	if(apartir_de_donde_escribir_relativo_a_bloque + *cuanto_escribir > super_bloque_info.block_size){
		int dist_fondo_bloque_apartir_de_donde_esc = super_bloque_info.block_size - apartir_de_donde_escribir_relativo_a_bloque;
		fwrite(copia_buffer, dist_fondo_bloque_apartir_de_donde_esc, 1, bloques);
		*cantidad_escrita += dist_fondo_bloque_apartir_de_donde_esc;
		*cuanto_escribir -= dist_fondo_bloque_apartir_de_donde_esc;
	}
	else {
		fwrite(copia_buffer, *cuanto_escribir, 1, bloques);
		*cantidad_escrita += *cuanto_escribir;
		*cuanto_escribir = 0;
	}
	free(copia_buffer);
	return;
}