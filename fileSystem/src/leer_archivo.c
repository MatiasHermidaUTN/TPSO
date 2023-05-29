#include "leer_archivo.h"

char* leer_archivo(char* nombre_archivo, int apartir_de_donde_leer, int cuanto_leer) {
	char* path = obtener_path_FCB_sin_free(nombre_archivo);
	t_config* archivo_FCB = iniciar_config(path);
	free(path);

	int cantidad_leida = 0; //para ir deplazandome en el buffer cuando ya leí
	//FAIL CHECKERS
	int tamanio_archivo = config_get_int_value(archivo_FCB, "TAMANIO_ARCHIVO");
	if (tamanio_archivo == 0){
		config_destroy(archivo_FCB);
		log_error(logger, "ERROR, tamanio_archivo == 0");
		return "";
	}
	if (apartir_de_donde_leer > tamanio_archivo) {
		log_error(logger, "ERROR, apartir_de_donde_leer es > al tamanio_archivo");
		return "";
	}
	if (apartir_de_donde_leer + cuanto_leer > tamanio_archivo) {
		cuanto_leer = tamanio_archivo - apartir_de_donde_leer;		//deberia tirar error?
		log_warning(logger, "leo hasta fin archivo\n");
	}

	//LECTURA
	const int cuanto_leer_cte = cuanto_leer;
	char* buffer = calloc(1, cuanto_leer_cte);
	//entro si leo de puntero_directo
	if (apartir_de_donde_leer < super_bloque_info.block_size){
		uint32_t puntero_directo = config_get_uint_value(archivo_FCB, "PUNTERO_DIRECTO");
		fseek(bloques, puntero_directo * super_bloque_info.block_size + apartir_de_donde_leer, SEEK_SET);

		leer_bloque(buffer, puntero_directo, apartir_de_donde_leer, &cuanto_leer, &cantidad_leida);
		leer_indirecto(buffer, archivo_FCB, PRIMER_BLOQUE_SECUNDARIO, &cuanto_leer, &cantidad_leida);
	}
	//entro si leo de puntero indirecto y cualquier puntero secundario
	else {
		int bloque_secundario_inicial = (int) ceil((float) apartir_de_donde_leer / super_bloque_info.block_size - 1);
		int apartir_de_donde_leer_bloque_inicial = apartir_de_donde_leer % super_bloque_info.block_size;
		apartir_de_donde_leer_bloque_inicial==0? bloque_secundario_inicial++ : bloque_secundario_inicial;	//pq si apartir es 0, el bloque inicial va a dar el anterior

		uint32_t puntero_indirecto = config_get_uint_value(archivo_FCB, "PUNTERO_INDIRECTO");
		uint32_t puntero_secundario = conseguir_ptr_secundario_para_indirecto(puntero_indirecto, bloque_secundario_inicial);

		leer_bloque(buffer, puntero_secundario, apartir_de_donde_leer_bloque_inicial, &cuanto_leer, &cantidad_leida);
		leer_indirecto(buffer, archivo_FCB, bloque_secundario_inicial+1, &cuanto_leer, &cantidad_leida);
	}
	config_destroy(archivo_FCB);
	return buffer;	//el free se hace en el switch, despues de pasarle el contenido a memoria
}

void leer_bloque(char* buffer, uint32_t puntero, int apartir_de_donde_leer_relativo_a_bloque, int* cuanto_leer, int* cantidad_leida){
	usleep(atoi(lectura_de_config.RETARDO_ACCESO_BLOQUE) * 1000);

	fseek(bloques, puntero * super_bloque_info.block_size + apartir_de_donde_leer_relativo_a_bloque, SEEK_SET);

	char* buffer1;
	if(apartir_de_donde_leer_relativo_a_bloque + *cuanto_leer > super_bloque_info.block_size){
		int cant_disponible_leer_de_puntero = super_bloque_info.block_size - apartir_de_donde_leer_relativo_a_bloque;

		buffer1 = calloc(1, cant_disponible_leer_de_puntero);
		fread(buffer1, cant_disponible_leer_de_puntero, 1, bloques);
		memcpy(buffer + *cantidad_leida, buffer1, cant_disponible_leer_de_puntero);

		*cantidad_leida += cant_disponible_leer_de_puntero;
		*cuanto_leer -= cant_disponible_leer_de_puntero;
	}
	else {
		buffer1 = calloc(1, *cuanto_leer);
		fread(buffer1, *cuanto_leer, 1, bloques);
		memcpy(buffer + *cantidad_leida, buffer1, *cuanto_leer);

		*cantidad_leida += *cuanto_leer;
		*cuanto_leer = 0;
	}
	free(buffer1);
	return;
}

void leer_indirecto(char* buffer, t_config* archivo_FCB, int bloque_secundario_donde_leer, int* cuanto_leer, int* cantidad_leida) {
	uint32_t puntero_indirecto = config_get_uint_value(archivo_FCB, "PUNTERO_INDIRECTO");
	uint32_t puntero_secundario;

	while(*cuanto_leer > 0) {
		puntero_secundario = conseguir_ptr_secundario_para_indirecto(puntero_indirecto, bloque_secundario_donde_leer);
		leer_bloque(buffer, puntero_secundario, DESDE_EL_INICIO, cuanto_leer, cantidad_leida);
		bloque_secundario_donde_leer++;
	}

}