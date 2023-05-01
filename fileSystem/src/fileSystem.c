#include "../include/fileSystem.h"
#define PRIMER_BLOQUE_SECUNDARIO 1
#define DESDE_EL_INICIO 0

t_config* superbloque;
//FILE* bitmap;
void* bitmap;
void* bitmap_pointer;
t_bitarray* bitarray_de_bitmap;
FILE* bloques;
//t_config* directorio_FCB;
int tamanioBitmap;

t_log* logger;
t_config* config;
t_fileSystem_config lectura_de_config;

int kernel;

int main(int argc, char** argv) {

	//LECTURA DE CONFIG DEL FILESYSTEM
	logger = iniciar_logger("FileSystem.log", "FS");
	config = iniciar_config("../fileSystem.config");

    lectura_de_config = leer_fileSystem_config(config);

	//CHQUEO DE QUE LOS PATHS A LOS DIFERENTES ARCHIVOS EXISTEN(SUPERBLOQUE, DIRECTORIO_FCB, BITMAP, BLOQUES)

	//SUPERBLOQUE
	if (archivo_se_puede_leer(lectura_de_config.PATH_SUPERBLOQUE)){
		superbloque = iniciar_config(lectura_de_config.PATH_SUPERBLOQUE);
		super_bloque_info.block_size = config_get_int_value(superbloque, "BLOCK_SIZE");
		super_bloque_info.block_count = config_get_int_value(superbloque, "BLOCK_COUNT");
		log_info(logger, "SuperBloque leido");
	} else {
		log_error(logger, "SuperBloque no esiste");
		return EXIT_FAILURE;
	}

	//BITMAP
	int fd = open(lectura_de_config.PATH_BITMAP, O_CREAT | O_RDWR, 0664);	//abre o crea el archivo en un file descriptor
	if (fd == -1) {
		close(fd);
		log_error(logger, "Error abriendo el bitmap");
		return EXIT_FAILURE;
	}
	double c = (double) super_bloque_info.block_count;
	tamanioBitmap = (int) ceil( c/8.0 ); 	// tener en cuenta si no se necesita en otro lado
	bitmap_pointer = mmap(NULL, tamanioBitmap, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	bitarray_de_bitmap = bitarray_create_with_mode((char*) bitmap_pointer, tamanioBitmap, MSB_FIRST);
	close(fd);
	log_info(logger, "Bitmap abierto");

	//BLOQUES
	bloques = fopen(lectura_de_config.PATH_BLOQUES, "r+");
	if (bloques){
		log_info(logger, "Bloques existe");

	} else {
		return EXIT_FAILURE;
		bloques = fopen(lectura_de_config.PATH_BLOQUES, "w+");
	}

	//**************TODO DEBUG**************
	//limpiar_bitmap();
	printf("unos inicial: %d\n", cant_unos_en_bitmap());
	/*for(unsigned long int i = 0 ; i < super_bloque_info.block_count * super_bloque_info.block_size ; i++){
		char j;
		if(i%32 < 16){
			j = i%16 + 97;
		}else{
			j = i%16 + 65;
		}
		fseek(bloques, i, SEEK_SET);
		fwrite(&j, sizeof(char), 1, bloques);
	}*/
	//**************TODO DEBUG**************

	//SE CONECTA AL SERIVDOR MEMORIA

	int socket_memoria = crear_conexion(lectura_de_config.IP_MEMORIA, lectura_de_config.PUERTO_MEMORIA);
	if(socket_memoria == -1){
		printf("No conectado a memoria\n");
	}
    enviar_handshake(socket_memoria, FILESYSTEM);

	//SE HACE SERVIDOR Y ESPERA LA CONEXION DEL KERNEL

	int server = iniciar_servidor("127.0.0.1", lectura_de_config.PUERTO_ESCUCHA);
	printf("Servidor listo para recibir al cliente, puerto: %s\n", lectura_de_config.PUERTO_ESCUCHA);
	kernel = esperar_cliente(server);
	if(kernel == -1){
		return -1;
	}
	puts("Se conecto el Kernel a FileSystem\n");


	while(1){
		t_instrucciones cod_op = recibir_cod_op(kernel);
		/*if(recibo_msj()){
			creo_hilo();
		}*/
		char* nombre_archivo;
		int nuevo_tamanio_archivo;
		int apartir_de_donde_X;
		int cuanto_X;
		int dir_fisica_memoria;
		char* buffer;

		switch (cod_op) {
			case ABRIR:{
				printf("abrir\n");
				recibir_parametros(cod_op, &nombre_archivo, &nuevo_tamanio_archivo, &apartir_de_donde_X, &cuanto_X, &dir_fisica_memoria);
				printf("nombre_archivo: %s\n",nombre_archivo);
				if (existe_archivo(nombre_archivo)) {	//existe FCB?
					enviar_mensaje_kernel(kernel, "OK El archivo ya existe");
					printf("existe/abierto %s\n",nombre_archivo);
				} else {
					enviar_mensaje_kernel(kernel, "ERROR El archivo NO existe");
					printf("no existe %s\n",nombre_archivo);
				}
				printf("\n");
				free(nombre_archivo);
				break;
			}
			case CREAR:{
				recibir_parametros(cod_op, &nombre_archivo, &nuevo_tamanio_archivo, &apartir_de_donde_X, &cuanto_X, &dir_fisica_memoria);
				printf("nombre_archivo: %s \n", nombre_archivo);
				printf("unos antes crear: %d\n", cant_unos_en_bitmap());
				crear_archivo(nombre_archivo);	//crear FCB y poner tamaño 0 y sin bloques asociados.
				enviar_mensaje_kernel(kernel, "OK Archivo creado");
				printf("archivo creado: %s\n",nombre_archivo);
				printf("unos dsp crear: %d\n", cant_unos_en_bitmap());
				printf("\n");
				free(nombre_archivo);
				break;
			}
			case TRUNCAR:{
				recibir_parametros(cod_op, &nombre_archivo, &nuevo_tamanio_archivo, &apartir_de_donde_X, &cuanto_X, &dir_fisica_memoria);
				printf("nombre_archivo: %s\n", nombre_archivo);
				printf("nuevo_tamanio_archivo: %d\n", nuevo_tamanio_archivo);
				printf("unos antes truncar: %d\n", cant_unos_en_bitmap());
				truncar(nombre_archivo, nuevo_tamanio_archivo);
				enviar_mensaje_kernel(kernel, "OK Archivo truncado");
				printf("unos dsp truncar: %d\n", cant_unos_en_bitmap());
				printf("\n");
				free(nombre_archivo);
				break;
			}
			case LEER:{
				recibir_parametros(cod_op, &nombre_archivo, &nuevo_tamanio_archivo, &apartir_de_donde_X, &cuanto_X, &dir_fisica_memoria);
				printf("nombre_archivo: %s\n", nombre_archivo);
				printf("apartir_de_donde_X: %d\n", apartir_de_donde_X);
				printf("cuanto_X: %d\n", cuanto_X);
				printf("dir_fisica_memoria: %d\n", dir_fisica_memoria);
				buffer = leer_archivo(nombre_archivo, apartir_de_donde_X, cuanto_X);	//malloc se hace en leer_archivo
				mandar_a_memoria(socket_memoria, ESCRIBIR, buffer, cuanto_X, dir_fisica_memoria);
				enviar_mensaje_kernel(kernel, "OK Archivo leido");
				printf("%s\n", buffer);
				printf("\n");
				free(buffer);
				free(nombre_archivo);
				break;
			}
			case ESCRIBIR:{
				recibir_parametros(cod_op, &nombre_archivo, &nuevo_tamanio_archivo, &apartir_de_donde_X, &cuanto_X, &dir_fisica_memoria);
				printf("nombre_archivo: %s\n", nombre_archivo);
				printf("apartir_de_donde_X: %d\n", apartir_de_donde_X);
				printf("cuanto_X: %d\n", cuanto_X);
				printf("dir_fisica_memoria: %d\n", dir_fisica_memoria);
				buffer = leer_de_memoria(socket_memoria, LEER, cuanto_X, dir_fisica_memoria);	//malloc se hace en leer_de_memoria
				escribir_archivo(buffer, nombre_archivo, apartir_de_donde_X, cuanto_X);
				enviar_mensaje_kernel(kernel, "OK Archivo escrito");
				puts(buffer);
				printf("\n");
				free(buffer);
				free(nombre_archivo);
				break;
			}
			case ERROR:
				//recibir_parametros(cod_op, &nombre_archivo, &nuevo_tamanio_archivo, &apartir_de_donde_X, &cuanto_X, &dir_fisica_memoria);
				break;
			default:
				break;
		}
	}
	config_destroy(superbloque);
	msync(bitmap_pointer, tamanioBitmap, MS_SYNC);
	munmap(bitmap_pointer, tamanioBitmap);
	fclose(bloques);
	return EXIT_SUCCESS;
}

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
		uint32_t puntero_secundario;
		fseek(bloques, puntero_indirecto * super_bloque_info.block_size + sizeof(puntero_secundario) * (bloque_secundario_inicial-1), SEEK_SET);
		fread(&puntero_secundario, sizeof(puntero_secundario), 1, bloques);

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
		fseek(bloques, puntero_indirecto * super_bloque_info.block_size + sizeof(puntero_secundario) * (bloque_secundario_donde_escribir-1), SEEK_SET);
		fread(&puntero_secundario, sizeof(puntero_secundario), 1, bloques);
		escribir_bloque(buffer, puntero_secundario, DESDE_EL_INICIO, cuanto_escribir, cantidad_escrita);
		bloque_secundario_donde_escribir++;
	}
	return;
}

void escribir_bloque(char* buffer, uint32_t puntero, int apartir_de_donde_escribir_relativo_a_bloque, int* cuanto_escribir, int* cantidad_escrita) {
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
	return;
}

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

	config_set_value(archivo_FCB, "TAMANIO_ARCHIVO", string_itoa(nuevo_tamanio_archivo));
	config_save_in_file(archivo_FCB, path);
	config_destroy(archivo_FCB);
	free(path);
	return;
}

uint32_t config_get_uint_value(t_config *self, char *key) {
	char *value = config_get_string_value(self, key);
	char *ptr;
	return (uint32_t) strtoul(value, &ptr, 10);
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
				uint32_t puntero;
				fseek(bloques, puntero_indirecto * super_bloque_info.block_size + sizeof(puntero) * (cant_punteros_secundarios-1), SEEK_SET);
				fread(&puntero, sizeof(puntero), 1, bloques);
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
			fseek(bloques, puntero_indirecto * super_bloque_info.block_size + sizeof(puntero) * cant_punteros_secundarios, SEEK_SET);
			fwrite(&puntero, sizeof(puntero), 1, bloques);				//anoto nuevo puntero_secundario en bloque de puntero_indirecto
		}
	}
	//entro si tiene puntero_directo, tiene puntero_indirecto (tiene 1 o + punteros_secundarios)
	if(tamanio_archivo > super_bloque_info.block_size){
		uint32_t puntero_indirecto = config_get_uint_value(archivo_FCB, "PUNTERO_INDIRECTO");

		int cant_punteros_secundarios = (int) ceil((float) tamanio_archivo / super_bloque_info.block_size - 1);
		int cant_bloques_secundarios_necesarios = (int) ceil((float) nuevo_tamanio_archivo / super_bloque_info.block_size - 1);
		for( ; cant_punteros_secundarios < cant_bloques_secundarios_necesarios ; cant_punteros_secundarios++) {
			uint32_t puntero = dame_un_bloque_libre();
			fseek(bloques, puntero_indirecto * super_bloque_info.block_size + sizeof(puntero) * cant_punteros_secundarios, SEEK_SET);
			fwrite(&puntero, sizeof(puntero), 1, bloques);				//anoto nuevo puntero_secundario en bloque de puntero_indirecto
		}
	}
	return;
}

void crear_archivo(char* nombre_archivo) {	//necesita semaforos para hilos pq entra a archivo comun FCBdefault
	char* path = obtener_path_FCB_sin_free(nombre_archivo);

	t_config* FCBdefault = iniciar_config("../FCBdefault");
	config_set_value(FCBdefault, "NOMBRE_ARCHIVO", nombre_archivo);
	config_set_value(FCBdefault, "TAMANIO_ARCHIVO", "0");
	config_set_value(FCBdefault, "PUNTERO_DIRECTO", "");
	config_set_value(FCBdefault, "PUNTERO_INDIRECTO", "");
	config_save_in_file(FCBdefault, path);
	config_destroy(FCBdefault);					//cierro el FCB
	free(path);
}

bool existe_archivo(char* nombre_archivo) {
	char* path = obtener_path_FCB_sin_free(nombre_archivo);
	bool retorno = archivo_se_puede_leer(path);
	free(path);
	return retorno;
}

char* obtener_path_FCB_sin_free(char* nombre_archivo){
	char* path = malloc(strlen(lectura_de_config.PATH_FCB) + strlen(nombre_archivo));
	strcpy(path, lectura_de_config.PATH_FCB);
	strcat(path, nombre_archivo);		//se asume que en la FS.config el path_fcb tiene / al final quedando: "/home/utnso/fs/fcb/"
	return path;
}

char* leer_archivo(char* nombre_archivo, int apartir_de_donde_leer, int cuanto_leer) {
	char* path = obtener_path_FCB_sin_free(nombre_archivo);
	t_config* archivo_FCB = iniciar_config(path);
	free(path);

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
		printf("leo hasta fin archivo\n");
	}

	//LECTURA
	const int cuanto_leer_cte = cuanto_leer;
	char* buffer = calloc(1, cuanto_leer_cte);
	strcpy(buffer, "");
	//entro si leo de puntero_directo
	if (apartir_de_donde_leer < super_bloque_info.block_size){
		uint32_t puntero_directo = config_get_uint_value(archivo_FCB, "PUNTERO_DIRECTO");
		fseek(bloques, puntero_directo * super_bloque_info.block_size + apartir_de_donde_leer, SEEK_SET);

		leer_bloque(buffer, puntero_directo, apartir_de_donde_leer, &cuanto_leer);
		leer_indirecto(buffer, archivo_FCB, PRIMER_BLOQUE_SECUNDARIO, &cuanto_leer);
	}
	//entro si leo de puntero indirecto y cualquier puntero secundario
	else {
		int bloque_secundario_inicial = (int) ceil((float) apartir_de_donde_leer / super_bloque_info.block_size - 1);
		int apartir_de_donde_leer_bloque_inicial = apartir_de_donde_leer % super_bloque_info.block_size;
		apartir_de_donde_leer_bloque_inicial==0? bloque_secundario_inicial++ : bloque_secundario_inicial;	//pq si apartir es 0, el bloque inicial va a dar el anterior

		uint32_t puntero_indirecto = config_get_uint_value(archivo_FCB, "PUNTERO_INDIRECTO");
		uint32_t puntero_secundario;
		fseek(bloques, puntero_indirecto * super_bloque_info.block_size + sizeof(puntero_secundario) * (bloque_secundario_inicial-1), SEEK_SET);
		fread(&puntero_secundario, sizeof(puntero_secundario), 1, bloques);

		leer_bloque(buffer, puntero_secundario, apartir_de_donde_leer_bloque_inicial, &cuanto_leer);
		leer_indirecto(buffer, archivo_FCB, bloque_secundario_inicial+1, &cuanto_leer);
	}
	config_destroy(archivo_FCB);
	char* buffer_aux = string_substring_until(buffer, cuanto_leer_cte);	//el malloc para 5 o menos char asigna memoria de mas con basura
	free(buffer);
	return buffer_aux;	//el free se hace en el switch, despues de pasarle el contenido a memoria
}

void leer_bloque(char* buffer, uint32_t puntero, int apartir_de_donde_leer_relativo_a_bloque, int* cuanto_leer){
	fseek(bloques, puntero * super_bloque_info.block_size + apartir_de_donde_leer_relativo_a_bloque, SEEK_SET);

	char* buffer1;
	char* buffer_aux;
	if(apartir_de_donde_leer_relativo_a_bloque + *cuanto_leer > super_bloque_info.block_size){
		int cant_disponible_leer_de_puntero = super_bloque_info.block_size - apartir_de_donde_leer_relativo_a_bloque;
		buffer1 = calloc(1, cant_disponible_leer_de_puntero);
		fread(buffer1, cant_disponible_leer_de_puntero, 1, bloques);
		buffer_aux = string_substring_until(buffer1, cant_disponible_leer_de_puntero);
		*cuanto_leer -= cant_disponible_leer_de_puntero;
	}
	else {
		buffer1 = calloc(1, *cuanto_leer);
		fread(buffer1, *cuanto_leer, 1, bloques);
		buffer_aux = string_substring_until(buffer1, *cuanto_leer);
		*cuanto_leer = 0;
	}
	strcat(buffer, buffer_aux);
	free(buffer1);
	free(buffer_aux);
	return;
}

void leer_indirecto(char* buffer, t_config* archivo_FCB, int bloque_secundario_donde_leer, int* cuanto_leer) {
	uint32_t puntero_indirecto = config_get_uint_value(archivo_FCB, "PUNTERO_INDIRECTO");
	uint32_t puntero_secundario;

	while(*cuanto_leer > 0) {
		fseek(bloques, puntero_indirecto * super_bloque_info.block_size + sizeof(puntero_secundario) * (bloque_secundario_donde_leer-1), SEEK_SET);
		fread(&puntero_secundario, sizeof(puntero_secundario), 1, bloques);
		leer_bloque(buffer, puntero_secundario, DESDE_EL_INICIO, cuanto_leer);
		bloque_secundario_donde_leer++;
	}

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

bool checkear_espacio(int cuanto_escribir) {
	int bloques_necesarios = cuanto_escribir / super_bloque_info.block_size;
	if (cuanto_escribir % super_bloque_info.block_size != 0)
		bloques_necesarios++;
	if (bloques_necesarios > bitarray_get_max_bit(bitarray_de_bitmap))
		return false;
	return true;
}

uint32_t dame_un_bloque_libre() {
	for (int i = 0; i < bitarray_get_max_bit(bitarray_de_bitmap); i++) {
		if (bitarray_test_bit(bitarray_de_bitmap, i) == 0) {
			bitarray_set_bit(bitarray_de_bitmap, i);
			msync(bitmap_pointer, tamanioBitmap, MS_SYNC);
			return i;
		}
	}
	return -1;	
}

//debug
int cant_unos_en_bitmap(){
	int contador = 0;
	for (int i = 0; i < bitarray_get_max_bit(bitarray_de_bitmap); i++) {
		if (bitarray_test_bit(bitarray_de_bitmap, i) == 1) {
			contador++;
		}
	}
	return contador;
}



t_instrucciones recibir_cod_op(int socket_cliente)
{
	t_instrucciones cod_op;
	if(recv(socket_cliente, &cod_op, sizeof(t_instrucciones), 0x100) > 0)
		return cod_op;
	else
	{
		close(socket_cliente);
		return ERROR;
	}
}

void recibir_parametros(t_instrucciones cod_op, char** nombre_archivo, int* tamanio_nuevo_archivo, int* apartir_de_donde_X, int* cuanto_X, int* dir_fisica_memoria){
	/*F_OPEN ARCHIVO
	F_TRUNCATE ARCHIVO 64
	F_WRITE ARCHIVO 4 4
	F_READ ARCHIVO 16 4*/
	size_t size_payload;
	if (recv(kernel, &size_payload, sizeof(size_t), 0) != sizeof(size_t)){
		return;
	}

	void* a_recibir = malloc(size_payload);
	if (recv(kernel, a_recibir, size_payload, 0) != size_payload) {
		free(a_recibir);
		return;
	}

	deserializar_instrucciones_kernel(a_recibir, size_payload, cod_op, nombre_archivo, tamanio_nuevo_archivo, apartir_de_donde_X, cuanto_X, dir_fisica_memoria);

	free(a_recibir);
	return;
}

void deserializar_instrucciones_kernel(void* a_recibir, int size_payload, t_instrucciones cod_op, char** nombre_archivo, int* tamanio_nuevo_archivo, int* apartir_de_donde_X, int* cuanto_X, int* dir_fisica_memoria){
	switch(cod_op){
		case ABRIR:
		case CREAR:
		default:
			int desplazamiento = 0;
			size_t largo_nombre;
			memcpy(&largo_nombre, a_recibir + desplazamiento, sizeof(largo_nombre));
			desplazamiento += sizeof(largo_nombre);
			//printf("largo_nombre: %d\n", largo_nombre);
			char* aux_nombre_file = malloc(largo_nombre);
			memcpy(aux_nombre_file, a_recibir + desplazamiento, largo_nombre);
			desplazamiento += largo_nombre;
			//printf("nombre_archivo: %s\n", aux_nombre_file);
			*nombre_archivo = strdup(aux_nombre_file);
			switch(cod_op){
				case TRUNCAR:
					memcpy(tamanio_nuevo_archivo, a_recibir + desplazamiento, sizeof(int));
					desplazamiento += sizeof(int);
					break;
				case LEER:
				case ESCRIBIR:
					memcpy(apartir_de_donde_X, a_recibir + desplazamiento, sizeof(int));
					desplazamiento += sizeof(int);
					memcpy(cuanto_X, a_recibir + desplazamiento, sizeof(int));
					desplazamiento += sizeof(int);
					memcpy(dir_fisica_memoria, a_recibir + desplazamiento, sizeof(int));
					desplazamiento += sizeof(int);
					break;
				case ABRIR:
				case CREAR:
				case ERROR:
					break;
			}
			break;
		case ERROR:
			break;
		}
}

void enviar_mensaje_kernel(int kernel, char* msj){
	return;
}
char* leer_de_memoria(int socket_memoria, t_instrucciones cod_op, int cuanto_escribir, int dir_fisica_memoria){
	char* buffer = malloc(cuanto_escribir);
	switch (cuanto_escribir){
	case 1:
		strcpy(buffer, "1");
		break;
	case 2:
		strcpy(buffer, "22");
		break;
	case 3:
		strcpy(buffer, "Que");
		break;
	case 4:
		strcpy(buffer, "bobo");
		break;
	case 5:
		strcpy(buffer, "miras");
		break;
	case 160:
		strcpy(buffer, "Atomic accelerators use 2 powerful magnets and electric fields to propel particles at high speeds, causing them to collide and release energy and radiation, XD.");
		break;
	default:
		strcpy(buffer, "hola");
		break;
	}
	return buffer;
}
void mandar_a_memoria(int socket_memoria, t_instrucciones cod_op, char* buffer, int cuanto_leer, int dir_fisica_memoria){
	return;
}

//debug
void limpiar_bitmap() {
	for (int i = 0; i < bitarray_get_max_bit(bitarray_de_bitmap); i++) {
		bitarray_clean_bit(bitarray_de_bitmap, i);
	}
	msync(bitmap_pointer, tamanioBitmap, MS_SYNC);
	return;
}

void liberar_bloque(uint32_t puntero){
	bitarray_clean_bit(bitarray_de_bitmap, puntero);
	msync(bitmap_pointer, tamanioBitmap, MS_SYNC);
}







