#include "../include/configuracion_fileSystem.h"

t_fileSystem_config leer_fileSystem_config(t_config* config) {

	t_fileSystem_config lectura_de_config;

	lectura_de_config.IP_MEMORIA            = config_get_string_value(config, "IP_MEMORIA");
	lectura_de_config.PUERTO_MEMORIA        = config_get_string_value(config, "PUERTO_MEMORIA");
	lectura_de_config.PUERTO_ESCUCHA        = config_get_string_value(config, "PUERTO_ESCUCHA");
	lectura_de_config.PATH_SUPERBLOQUE      = config_get_string_value(config, "PATH_SUPERBLOQUE");
	lectura_de_config.PATH_BITMAP           = config_get_string_value(config, "PATH_BITMAP");
	lectura_de_config.PATH_BLOQUES          = config_get_string_value(config, "PATH_BLOQUES");
	lectura_de_config.PATH_FCB              = config_get_string_value(config, "PATH_FCB");
	lectura_de_config.RETARDO_ACCESO_BLOQUE = config_get_string_value(config, "RETARDO_ACCESO_BLOQUE");

	return lectura_de_config;
}

void init_superbloque() {
	//SUPERBLOQUE
	if (archivo_se_puede_leer(lectura_de_config.PATH_SUPERBLOQUE)){
		superbloque = iniciar_config(lectura_de_config.PATH_SUPERBLOQUE);
		super_bloque_info.block_size = config_get_int_value(superbloque, "BLOCK_SIZE");
		super_bloque_info.block_count = config_get_int_value(superbloque, "BLOCK_COUNT");
		log_warning(my_logger, "SuperBloque leido");
	}
	else {
		log_error(logger, "SuperBloque no existe");
		//destroy everything in reality
	}
}

void init_bitmap() {
	//BITMAP
	tamanioBitmap = (int) ceil( (double) super_bloque_info.block_count/8.0 );
	int fd_bitmap = open(lectura_de_config.PATH_BITMAP, O_RDWR, 0664);	//abre o crea el archivo en un file descriptor
	if (fd_bitmap == -1) {
		close(fd_bitmap);
		log_warning(my_logger, "Creando bitmap");
		fd_bitmap = open(lectura_de_config.PATH_BITMAP, O_CREAT | O_RDWR, 0664); //abre o crea el archivo en un file descriptor
		if (fd_bitmap == -1){
			log_error(logger, "No se pudo ni CREAR el bitmap");
			//destroy everything in reality
		}
		ftruncate(fd_bitmap, tamanioBitmap+10);
	}
	bitmap_pointer = mmap(NULL, tamanioBitmap, PROT_READ | PROT_WRITE, MAP_SHARED, fd_bitmap, 0);
	bitarray_de_bitmap = bitarray_create_with_mode((char*) bitmap_pointer, tamanioBitmap, MSB_FIRST);
	close(fd_bitmap);
	log_warning(my_logger, "Bitmap abierto");
}

void init_bloques() {
	//BLOQUES
	bloques = fopen(lectura_de_config.PATH_BLOQUES, "r+");
	if (bloques == NULL) {
		log_warning(my_logger, "CREANDO BLOQUES");
		int fd_bloques = open(lectura_de_config.PATH_BLOQUES, O_CREAT | O_RDWR, 0664);	//abre o crea el archivo en un file descriptor
		if (fd_bloques == -1){
			log_error(logger, "No se pudo ni CREAR el bloques");
			//destroy everything in reality
		}
		int tamanioBloques = super_bloque_info.block_size * super_bloque_info.block_count;
		ftruncate(fd_bloques, tamanioBloques+10);
		close(fd_bloques);
		bloques = fopen(lectura_de_config.PATH_BLOQUES, "r+");
	}
	log_warning(my_logger, "Bloques abierto");
}

void init_carpeta_fcbs() {
	//FCBs carpeta raiz
	char* comando = malloc(10 + strlen("mkdir -v -p ") + strlen(lectura_de_config.PATH_FCB));
	strcpy(comando, "mkdir -v -p ");
	strcat(comando, lectura_de_config.PATH_FCB);
	system(comando);
	free(comando);
}
