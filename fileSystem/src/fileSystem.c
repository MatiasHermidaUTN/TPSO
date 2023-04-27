#include "../include/fileSystem.h"
#define PRIMER_BLOQUE_SECUNDARIO 1
#define LEER_DESDE_EL_INICIO 0

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

	//SE CONECTA AL SERIVDOR MEMORIA
/*
	int socket_memoria = crear_conexion(lectura_de_config.IP_MEMORIA, lectura_de_config.PUERTO_MEMORIA);
    enviar_handshake(socket_memoria, FILESYSTEM);
*/
	//SE HACE SERVIDOR Y ESPERA LA CONEXION DEL KERNEL
	//TODO CREAR UN HILO PARA ESPERAR EL CLIENTE(KERNEL)

	int server = iniciar_servidor("127.0.0.1", lectura_de_config.PUERTO_ESCUCHA);
	puts("Servidor listo para recibir al cliente");
	kernel = esperar_cliente(server);

	puts("Se conecto el Kernel a FileSystem");

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

	limpiar_bitmap();

	while(1){
		t_instrucciones cod_op = recibir_cod_op(kernel);
		//recv(kernel, &cod_op, sizeof(t_instrucciones), 0);
		char* nombre_archivo;// = strdup("archivo1");
		int nuevo_tamanio_archivo;
		int apartir_de_donde_X;
		int cuanto_X;
		int dir_fisica_memoria;
		char* buffer;

		switch (cod_op) {
			case ABRIR:{
				printf("abrir\n");
				recibir_parametros(cod_op, &nombre_archivo, &nuevo_tamanio_archivo, &apartir_de_donde_X, &cuanto_X, &dir_fisica_memoria);
				printf("%s\n",nombre_archivo);
				if (existe_archivo(nombre_archivo)) {	//existe FCB?
					//enviar_mensaje_kernel(kernel, "OK El archivo ya existe");
					printf("existe/abierto %s\n",nombre_archivo);
				} else {
					//enviar_mensaje_kernel(kernel, "ERROR El archivo NO existe");
					printf("no existe %s\n",nombre_archivo);
				}
				free(nombre_archivo);
				break;
			}
			case CREAR:{
				recibir_parametros(cod_op, &nombre_archivo, &nuevo_tamanio_archivo, &apartir_de_donde_X, &cuanto_X, &dir_fisica_memoria);
				printf("nombre_archivo: %s \n", nombre_archivo);
				crear_archivo(nombre_archivo);	//crear FCB y poner tamaño 0 y sin bloques asociados.
				enviar_mensaje_kernel(kernel, "OK Archivo creado");
				printf("archivo creado: %s\n",nombre_archivo);
				free(nombre_archivo);
				break;
			}
			case TRUNCAR:{
				recibir_parametros(cod_op, &nombre_archivo, &nuevo_tamanio_archivo, &apartir_de_donde_X, &cuanto_X, &dir_fisica_memoria);
				printf("nombre_archivo: %s\n", nombre_archivo);
				printf("nuevo_tamanio_archivo: %d\n", nuevo_tamanio_archivo);
				truncar(nombre_archivo, nuevo_tamanio_archivo);
				//enviar_mensaje_kernel(kernel, "OK Archivo truncado");
				free(nombre_archivo);
				break;
			}
			case LEER:{
				recibir_parametros(cod_op, &nombre_archivo, &nuevo_tamanio_archivo, &apartir_de_donde_X, &cuanto_X, &dir_fisica_memoria);
				printf("nombre_archivo: %s\n", nombre_archivo);
				printf("apartir_de_donde_X: %d\n", apartir_de_donde_X);
				printf("cuanto_X: %d\n", cuanto_X);
				printf("dir_fisica_memoria: %d\n", dir_fisica_memoria);
				//buffer = leer_archivo(nombre_archivo, apartir_de_donde_X, cuanto_X);	//malloc se hace en leer_archivo
				//mandar_a_memoria(socket_memoria, ESCRIBIR, buffer, cuanto_X, dir_fisica_memoria);
				//enviar_mensaje_kernel(kernel, "OK Archivo leido");
				//free(buffer);
				free(nombre_archivo);
				break;
			}
			case ESCRIBIR:{
				recibir_parametros(cod_op, &nombre_archivo, &nuevo_tamanio_archivo, &apartir_de_donde_X, &cuanto_X, &dir_fisica_memoria);
				printf("nombre_archivo: %s\n", nombre_archivo);
				printf("apartir_de_donde_X: %d\n", apartir_de_donde_X);
				printf("cuanto_X: %d\n", cuanto_X);
				printf("dir_fisica_memoria: %d\n", dir_fisica_memoria);
				//buffer = leer_de_memoria(socket_memoria, LEER, cuanto_X, dir_fisica_memoria);	//malloc se hace en leer_de_memoria
				//escribir_archivo(buffer, nombre_archivo, apartir_de_donde_X, cuanto_X);
				//enviar_mensaje_kernel(kernel, "OK Archivo escrito");
				//free(buffer);
				free(nombre_archivo);
				break;
			}
			case ERROR:
				//recibir_parametros(cod_op, &nombre_archivo, &nuevo_tamanio_archivo, &apartir_de_donde_X, &cuanto_X, &dir_fisica_memoria);
				break;
			default:
				break;
		}
		msync(bitmap_pointer, tamanioBitmap, MS_SYNC);
	}
	config_destroy(superbloque);
	msync(bitmap_pointer, tamanioBitmap, MS_SYNC);
	munmap(bitmap_pointer, tamanioBitmap);
	fclose(bloques);
	return EXIT_SUCCESS;
}

void truncar(char* nombre_archivo, int nuevo_tamanio_archivo){
	char* path = obtener_path_FCB_sin_free(nombre_archivo);
	t_config* archivo_FCB = iniciar_config(path);
	free(path);

	int tamanio_archivo = config_get_int_value(archivo_FCB, "TAMANIO_ARCHIVO");

	if (nuevo_tamanio_archivo > tamanio_archivo){
		agrandas_archivo(archivo_FCB, nombre_archivo, nuevo_tamanio_archivo);
	}
	else if (nuevo_tamanio_archivo < tamanio_archivo) {
		achicas_archivo(archivo_FCB, nombre_archivo, nuevo_tamanio_archivo);
	}
	config_set_value(archivo_FCB, "TAMANIO_ARCHIVO", string_itoa(nuevo_tamanio_archivo));
	config_destroy(archivo_FCB);
}

void achicas_archivo(t_config* archivo_FCB, char* nombre_archivo, int nuevo_tamanio_archivo){
	int tamanio_archivo = config_get_int_value(archivo_FCB, "TAMANIO_ARCHIVO");

	//entro si tiene puntero_directo y ya tiene puntero_indirecto
	if(tamanio_archivo > super_bloque_info.block_size){
		uint32_t puntero_indirecto = config_get_int_value(archivo_FCB, "PUNTERO_INDIRECTO");

		int cant_punteros_secundarios = (int) ceil((float) tamanio_archivo / super_bloque_info.block_size - 1);
		int cant_bloques_secundarios_necesarios = (int) ceil((float) nuevo_tamanio_archivo / super_bloque_info.block_size - 1);
		if(cant_bloques_secundarios_necesarios < cant_punteros_secundarios){
			for( ; cant_punteros_secundarios > cant_bloques_secundarios_necesarios ; ) {
				cant_punteros_secundarios--;	//lo pongo aca para el seek el 1er ptr_secundario esta en [0]
				uint32_t puntero;
				fseek(bloques, puntero_indirecto * super_bloque_info.block_size + sizeof(puntero) * cant_punteros_secundarios, SEEK_SET);
				fread(puntero, sizeof(puntero), 1, bloques);
				liberar_bloque(puntero);
			}
		}
		if(nuevo_tamanio_archivo <= super_bloque_info.block_size){
			uint32_t puntero_indirecto = config_get_int_value(archivo_FCB, "PUNTERO_INDIRECTO");
			liberar_bloque(puntero_indirecto);
			config_set_value(archivo_FCB, "PUNTERO_INDIRECTO", "");
		}
	}
	if(nuevo_tamanio_archivo == 0){
		uint32_t puntero_directo = config_get_int_value(archivo_FCB, "PUNTERO_DIRECTO");
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
		uint32_t puntero_indirecto = config_get_int_value(archivo_FCB, "PUNTERO_INDIRECTO");

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
	
/*void delete_FCB(char* nombre_archivo) {
	char* path;
	strcpy(path, lectura_de_config.PATH_FCB);
	strcat(path, nombre_archivo);		//se asume que en la FS.config el path_fcb tiene / al final quedando: "/home/utnso/fs/fcb/"
	remove(path);
}*/ //hace falta esto???

char* leer_archivo(char* nombre_archivo, int apartir_de_donde_leer, int cuanto_leer) {
	char* path = obtener_path_FCB_sin_free(nombre_archivo);
	t_config* archivo_FCB = iniciar_config(path);
	free(path);

	//FAIL CHECKERS
	int tamanio_archivo = config_get_int_value(archivo_FCB, "TAMANIO_ARCHIVO");
	if (apartir_de_donde_leer > tamanio_archivo) {
		return "ERROR, apartir_de_donde_leer es > al tamanio_archivo";
	}
	if (apartir_de_donde_leer + cuanto_leer > tamanio_archivo) {
		cuanto_leer = tamanio_archivo - apartir_de_donde_leer;		//deberia tirar error?
		//return "ERROR, apartir_de_donde_leer + cuanto_leer es > al tamanio_archivo";
	}

	//LECTURA
	char* buffer = malloc(cuanto_leer);
	if (tamanio_archivo != 0) {
		//entro si leo de puntero_directo
		if (apartir_de_donde_leer < super_bloque_info.block_size){
			uint32_t puntero_directo = config_get_int_value(archivo_FCB, "PUNTERO_DIRECTO");
			fseek(bloques, puntero_directo * super_bloque_info.block_size, SEEK_SET);
			int cant_disponible_leer_de_puntero = super_bloque_info.block_size - apartir_de_donde_leer;

			if(cuanto_leer < cant_disponible_leer_de_puntero){
				fread(buffer, cuanto_leer, 1, bloques);
				cuanto_leer = 0;
			}else{
				fread(buffer, cant_disponible_leer_de_puntero, 1, bloques);
				cuanto_leer -= cant_disponible_leer_de_puntero;
				leer_indirecto(&buffer, archivo_FCB, PRIMER_BLOQUE_SECUNDARIO, LEER_DESDE_EL_INICIO, cuanto_leer);
			}
		}
		//entro si leo de puntero indirecto y cualquier puntero secundario
		if (apartir_de_donde_leer >= super_bloque_info.block_size){
			int bloque_secundario_inicial = (int) ceil((float) apartir_de_donde_leer / super_bloque_info.block_size - 1);
			int apartir_de_donde_leer_relativo_a_bloque = apartir_de_donde_leer%super_bloque_info.block_size;
			leer_indirecto(&buffer, archivo_FCB, bloque_secundario_inicial, apartir_de_donde_leer_relativo_a_bloque, cuanto_leer);
		}
	}
	else {
		//logear error
		//"No puedo leer archivo vacio"?
		config_destroy(archivo_FCB);
		exit(EXIT_FAILURE);
	}
	config_destroy(archivo_FCB);
	return buffer;	//el free se hace en el switch, despues de pasarle el contenido a memoria
}

void leer_indirecto(char** buffer, t_config* archivo_FCB, int bloque_secundario_donde_leer, int apartir_de_donde_leer_relativo_a_bloque, int cuanto_leer) {
	int tamanio_archivo = config_get_int_value(archivo_FCB, "TAMANIO_ARCHIVO");
	uint32_t puntero_indirecto = config_get_int_value(archivo_FCB, "PUNTERO_INDIRECTO");
	uint32_t puntero_secundario;
	fseek(bloques, puntero_indirecto * super_bloque_info.block_size + sizeof(puntero_secundario) * bloque_secundario_donde_leer, SEEK_SET);
	fread(&bloque_secundario_donde_leer, sizeof(puntero_secundario), 1, bloques);				//anoto nuevo puntero_secundario en bloque de puntero_indirecto

	int cant_disponible_leer_de_puntero = super_bloque_info.block_size - apartir_de_donde_leer_relativo_a_bloque;
	if(cuanto_leer < cant_disponible_leer_de_puntero){
		char * buffer1 = malloc(cuanto_leer);
		fread(buffer1, cuanto_leer, 1, bloques);
		cuanto_leer = 0;
		strcat(*buffer, buffer1);
		free(buffer1);
		strcat(*buffer, buffer1);
	}else{
		char * buffer1 = malloc(cant_disponible_leer_de_puntero);
		fread(buffer1, cant_disponible_leer_de_puntero, 1, bloques);
		cuanto_leer -= cant_disponible_leer_de_puntero;
		strcat(*buffer, buffer1);
		free(buffer1);
		leer_indirecto(&buffer, archivo_FCB, bloque_secundario_donde_leer+1, LEER_DESDE_EL_INICIO, cuanto_leer);
	}
/*
	int cant_punteros_secundarios = (int) ceil((float) tamanio_archivo / super_bloque_info.block_size - 1);
	int cant_bloques_secundarios_necesarios = (int) ceil((float) (apartir_de_donde_leer+cuanto_leer) / super_bloque_info.block_size - 1);

	for(int bloque_secundario_a_leer = bloque_secundario_inicial-1 ; bloque_secundario_a_leer < cant_bloques_secundarios_necesarios ; bloque_secundario_a_leer++) {
		uint32_t puntero;
		fseek(bloques, puntero_indirecto * super_bloque_info.block_size + sizeof(puntero) * bloque_secundario_a_leer, SEEK_SET);
		fread(&puntero, sizeof(puntero), 1, bloques);				//anoto nuevo puntero_secundario en bloque de puntero_indirecto

		char * buffer1 = malloc(super_bloque_info.block_size);
		int cant_disponible_leer_de_puntero = super_bloque_info.block_size - apartir_de_donde_leer;

		fseek(bloques, puntero * super_bloque_info.block_size, SEEK_SET);
		if(cuanto_leer >= super_bloque_info.block_size){
			fread(buffer1, super_bloque_info.block_size, 1, bloques);
		} else {
			fread(buffer1, cuanto_leer, 1, bloques);
		}
		strcat(*buffer, buffer1);
		if(cuanto_leer < cant_disponible_leer_de_puntero){
			fread(buffer, cuanto_leer, 1, bloques);
			cuanto_leer = 0;
		}else{
			fread(buffer, cant_disponible_leer_de_puntero, 1, bloques);
			cuanto_leer -= cant_disponible_leer_de_puntero;
			//leer_indirecto(&buffer, archivo_FCB, super_bloque_info.block_size+1, cuanto_leer);
		}
		free(buffer1);
	}


	uint32_t dir_bloque;



	t_list* aux = list_create();
	t_list_iterator *lista_bloques = list_iterator_create(aux);	//TODO Fijarse que probablemente este MAL

	int cantidad_de_bloques = (int) ceil((cuanto_leer - super_bloque_info.block_size)/super_bloque_info.block_size);
	
	//agarrar direccion a los bloques de contenido desde el bloque de puntero indirecto
	fseek(bloques, puntero_indirecto * super_bloque_info.block_size, SEEK_SET);
	for(int i = 0; i < cantidad_de_bloques; i++){
		fseek(bloques, puntero_indirecto * super_bloque_info.block_size + i * sizeof(uint32_t), SEEK_SET);
		fread(&dir_bloque, sizeof(uint32_t), 1, bloques);
		list_iterator_add(lista_bloques, &dir_bloque);
	}

	//lees los bloques de contenido, teniendo en cuenta que el ultimo bloque podria
	//no tener q leerse por completo
	dir_bloque = list_iterator_index(0);
	while (true) {
		fseek(bloques, dir_bloque * super_bloque_info.block_size, SEEK_SET);
		if(cuanto_leer >= super_bloque_info.block_size){
			//fread(buffer1, super_bloque_info.block_size, 1, bloques);
		} else {
			//fread(buffer1, cuanto_leer, 1, bloques);
		}
		//strcat(*buffer, buffer1);

		cuanto_leer -= super_bloque_info.block_size;
		if(list_iterator_has_next(lista_bloques))
			dir_bloque = (uint32_t) list_iterator_next(lista_bloques);		//TODO
		else
			break;
	}
	//free(buffer1);*/
}

void escribir_archivo(char* buffer, char* nombre_archivo, int apartir_de_donde_escribir, int cuanto_escribir) {
	char* cuanto_escribir_string, puntero_directo_string, puntero_indirecto_string;
	int cantidad_escrita = 0;


	char* path = obtener_path_FCB_sin_free(nombre_archivo);
	t_config* archivo_FCB = iniciar_config(path);
	free(path);
	
	int tamanio_archivo = config_get_int_value(archivo_FCB, "TAMANIO_ARCHIVO");

	if(checkear_espacio(cuanto_escribir))//chequea que pueda escribi todo!!!!
	{
		if (tamanio_archivo == 0){		//solo para archivos NUEVOS
			cuanto_escribir_string = string_itoa(cuanto_escribir);
			config_set_value(archivo_FCB, "TAMANIO_ARCHIVO", cuanto_escribir_string);

			uint32_t puntero_directo = dame_un_bloque_libre();
			puntero_directo_string = string_itoa(puntero_directo);
			config_set_value(archivo_FCB, "PUNTERO_DIRECTO", puntero_directo_string);
			escribir_bloque(&buffer, puntero_directo, &cuanto_escribir, &cantidad_escrita, super_bloque_info.block_size);
			
			if(cuanto_escribir > 0){
				uint32_t puntero_indirecto = dame_un_bloque_libre();
				puntero_indirecto_string = string_itoa(puntero_indirecto);
				config_set_value(archivo_FCB, "PUNTERO_INDIRECTO", puntero_indirecto_string);

				for(int cant_punteros_secundarios = 0 ; cuanto_escribir > 0 ; cant_punteros_secundarios++) {
					uint32_t puntero = dame_un_bloque_libre();
					fseek(bloques, puntero_indirecto * super_bloque_info.block_size + sizeof(puntero) * cant_punteros_secundarios, SEEK_SET);
					fwrite(puntero, sizeof(puntero), 1, bloques);				//anoto nuevo puntero_secundario en bloque de puntero_indirecto
					escribir_bloque(&buffer, puntero, &cuanto_escribir, &cantidad_escrita, super_bloque_info.block_size);
				}
			}
		}
		else {
			uint32_t puntero_directo = (uint32_t) config_get_int_value(archivo_FCB, "PUNTERO_DIRECTO");
			uint32_t puntero_indirecto = (uint32_t) config_get_int_value(archivo_FCB, "PUNTERO_INDIRECTO");

			cuanto_escribir_string = intToCharAsterisco((unsigned int) (cuanto_escribir + tamanio_archivo));
			config_set_value(archivo_FCB, "TAMANIO_ARCHIVO", cuanto_escribir_string);

			int cantidad_de_bloques_ocupados = (tamanio_archivo / super_bloque_info.block_size);	//ej: 120 / 64 = 1 (trunca)
			if(cantidad_de_bloques_ocupados == 0) {
				escribir_bloque(&buffer, puntero_directo, &cuanto_escribir, &cantidad_escrita, super_bloque_info.block_size);
			}
			else {
				uint32_t puntero;
				cantidad_de_bloques_ocupados--;	//resto el bloque correspondiente al puntero directo
				fseek(bloques, puntero_indirecto * super_bloque_info.block_size + sizeof(puntero) * cantidad_de_bloques_ocupados, SEEK_SET);
				fread(&puntero, sizeof(puntero), 1, bloques);
				
				int espacio_libre_en_bloque = tamanio_archivo - cantidad_de_bloques_ocupados * super_bloque_info.block_size;
				escribir_bloque(&buffer, puntero, &cuanto_escribir, &cantidad_escrita, espacio_libre_en_bloque);
				cantidad_de_bloques_ocupados++;

				for(int cant_punteros_secundarios = cantidad_de_bloques_ocupados ; cuanto_escribir > 0 ; cant_punteros_secundarios++) {
					uint32_t puntero = dame_un_bloque_libre();
					fseek(bloques, puntero_indirecto * super_bloque_info.block_size + sizeof(puntero) * cant_punteros_secundarios, SEEK_SET);
					fwrite(puntero, sizeof(puntero), 1, bloques);				//anoto nuevo puntero_secundario en bloque de puntero_indirecto
					escribir_bloque(&buffer, puntero, &cuanto_escribir, &cantidad_escrita, super_bloque_info.block_size);
				}
			}
		}
	}
	
	free(cuanto_escribir_string);
	free(puntero_directo_string);
	free(puntero_indirecto_string);
	config_destroy(archivo_FCB);
	
	/*while (cuanto_escribir != 0) {
		fseek(bloques, puntero_directo * super_bloque_info.block_size, SEEK_SET);
		if(cuanto_escribir < super_bloque_info.block_size){
			fwrite(buffer, cuanto_escribir, 1, bloques);
		}else{
			fwrite(buffer, super_bloque_info.block_size, 1, bloques);
			escribir_indirecto(&buffer, puntero_indirecto, cuanto_escribir);
		}
	}
	else {
		//logear error
		exit(EXIT_FAILURE);
	}*/
}

bool archivo_se_puede_leer(char* path)
{
	FILE* f;
	if(f = fopen(path, "r")){
		fclose(f);
		return 1;
	} else {
		return 0;
	}
	//return (!access(path, R_OK ));
}

bool archivo_se_puede_escribir(char* path)
{
	return (!access(path, W_OK ));
}

//Prueba int to char*
// seguramente haya q pasar por referencia el char*
char* intToCharAsterisco(unsigned int numero) {

	numero = abs(numero);

	int digitos = cantidad_de_digitos(numero);

	char* string = malloc(digitos + 1);
	int digito;

	if(numero == 0)
		return "0";

	for (int i = 0, j = digitos - 1, copiaNumero = numero; numero > 0; i++, j--) {
        copiaNumero /= 10;
        digito = numero - (copiaNumero * 10);
        string[j] = digito  + '0';
        numero = copiaNumero;
    }

    return string;
}

int cantidad_de_digitos(unsigned int numero) { 
  int cociente = 0, divisor = 1, contador = 0;

  while (cociente !=1){
    contador++;
    divisor = divisor * 10;
    cociente = numero / divisor;

    if (cociente == 1){
    	return contador + 1;
    }

    if (cociente == 0){
    	return contador;
    }

    if (numero < 10){
    	return 1;
    }
  }
  return 0;
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

void escribir_bloque(char* buffer, uint32_t puntero, int* cuanto_escribir, int* cantidad_escrita, int espacio_libre_en_bloque) {
	int a_partir_de_donde_escribir = super_bloque_info.block_size - espacio_libre_en_bloque;
	fseek(bloques, puntero * super_bloque_info.block_size + a_partir_de_donde_escribir, SEEK_SET);

	char* copia_buffer = (char*) string_substring(buffer, (int)*cantidad_escrita, (int)*cuanto_escribir);

	if(*cuanto_escribir < espacio_libre_en_bloque){
		fwrite(copia_buffer, *cuanto_escribir, 1, bloques);
		*cantidad_escrita += *cuanto_escribir;
		*cuanto_escribir = 0;
	}
	else {
		fwrite(copia_buffer, espacio_libre_en_bloque, 1, bloques);
		*cantidad_escrita += espacio_libre_en_bloque;
		*cuanto_escribir -= espacio_libre_en_bloque;
	}
	free(copia_buffer);
}

int espacio_libre_en_bloque(uint32_t puntero) {
	int espacio_libre = 0;
	fseek(bloques, puntero * super_bloque_info.block_size, SEEK_SET);
	char* buffer = malloc(super_bloque_info.block_size);
	fread(buffer, super_bloque_info.block_size, 1, bloques);
	for (int i = 0; i < super_bloque_info.block_size; i++) {
		if (buffer[i] == '\0')
			espacio_libre++;
	}
	free(buffer);
	return espacio_libre;
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
	size_t largo_nombre;

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
			}
			break;
		case ERROR:
			break;
		}
}

void enviar_mensaje_kernel(int kernel, char* msj){
	return;
}
char* leer_de_memoria(int socket_memoria, t_instrucciones LEER, int cuanto_escribir, int dir_fisica_memoria){
	return "hey";
}
void mandar_a_memoria(int socket_memoria, t_instrucciones ESCRIBIR, char* buffer, int cuanto_leer, int dir_fisica_memoria){
	return;
}







