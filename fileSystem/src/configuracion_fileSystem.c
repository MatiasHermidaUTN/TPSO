#include "../include/configuracion_fileSystem.h"

t_config* superbloque;
void* bitmap;
void* bitmap_pointer;
t_bitarray* bitarray_de_bitmap;
FILE* bloques;
int tamanioBitmap;

t_log* logger;
t_config* config;
t_fileSystem_config lectura_de_config;

int socket_kernel;
int socket_memoria;

pthread_mutex_t mutex_cola_msj;
sem_t sem_sincro_cant_msj;

t_list* lista_fifo_msj;

t_super_bloque_info super_bloque_info;

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
	if (archivo_se_puede_leer(lectura_de_config.PATH_SUPERBLOQUE)) {
		superbloque = iniciar_config(lectura_de_config.PATH_SUPERBLOQUE);
		super_bloque_info.block_size = config_get_int_value(superbloque, "BLOCK_SIZE");
		super_bloque_info.block_count = config_get_int_value(superbloque, "BLOCK_COUNT");
		log_info(logger, "SuperBloque leido");
	}
	else {
		log_error(logger, "SuperBloque no existe");
		//destroy everything in reality
		exit(EXIT_FAILURE);
	}
}

void init_bitmap() {
	//BITMAP
	tamanioBitmap = (int) ceil( (double) super_bloque_info.block_count/8.0 );
	int fd_bitmap = open(lectura_de_config.PATH_BITMAP, O_RDWR, 0664);	//abre o crea el archivo en un file descriptor
	if (fd_bitmap == -1) {
		close(fd_bitmap);
		log_warning(logger, "Creando bitmap");
		fd_bitmap = open(lectura_de_config.PATH_BITMAP, O_CREAT | O_RDWR, 0664); //abre o crea el archivo en un file descriptor
		if (fd_bitmap == -1){
			log_error(logger, "No se pudo ni CREAR el bitmap");
			//destroy everything in reality
			exit(EXIT_FAILURE);
		}
		ftruncate(fd_bitmap, tamanioBitmap+10);
	}
	bitmap_pointer = mmap(NULL, tamanioBitmap, PROT_READ | PROT_WRITE, MAP_SHARED, fd_bitmap, 0);
	bitarray_de_bitmap = bitarray_create_with_mode((char*) bitmap_pointer, tamanioBitmap, MSB_FIRST);
	close(fd_bitmap);
	log_info(logger, "Bitmap abierto");
}

void init_bloques() {
	//BLOQUES
	bloques = fopen(lectura_de_config.PATH_BLOQUES, "r+");
	if (bloques == NULL) {
		log_warning(logger, "CREANDO BLOQUES");
		int fd_bloques = open(lectura_de_config.PATH_BLOQUES, O_CREAT | O_RDWR, 0664);	//abre o crea el archivo en un file descriptor
		if (fd_bloques == -1){
			log_error(logger, "No se pudo ni CREAR el bloques");
			//destroy everything in reality
			exit(EXIT_FAILURE);
		}
		int tamanioBloques = super_bloque_info.block_size * super_bloque_info.block_count;
		ftruncate(fd_bloques, tamanioBloques+10);
		close(fd_bloques);
		bloques = fopen(lectura_de_config.PATH_BLOQUES, "r+");
	}
	log_info(logger, "Bloques abierto");
}

void init_carpeta_fcbs() {
	//FCBs carpeta raiz
	char* comando = malloc(10 + strlen("mkdir -v -p ") + strlen(lectura_de_config.PATH_FCB));
	strcpy(comando, "mkdir -v -p ");
	strcat(comando, lectura_de_config.PATH_FCB);
	system(comando);
	free(comando);
}

//comunicaciones
char* leer_de_memoria(int cuanto_X, int dir_fisica_memoria){
	char ** parametros_a_enviar = string_array_new();
	string_array_push(&parametros_a_enviar, string_itoa(dir_fisica_memoria));
	string_array_push(&parametros_a_enviar, string_itoa(cuanto_X));
	enviar_msj_con_parametros(socket_kernel, LEER_ARCHIVO, parametros_a_enviar);
	free(parametros_a_enviar);

	char* buffer = malloc(cuanto_X);
	t_mensajes* mensaje = malloc(sizeof(t_mensajes));

	mensaje->cod_op = recibir_msj(socket_memoria);
	//if(mensaje->cod_op != LEIDO) //debería ser LEIDO??? habría que hacer un t_msj_fileSystem_memoria
	//		;//kaboom??? TODO

	mensaje->parametros = recibir_parametros_de_mensaje(socket_memoria);

	memcpy(buffer, mensaje->parametros[0], cuanto_X);
	string_array_destroy(mensaje->parametros);

	return buffer;
}

void escribir_en_memoria(int dir_fisica_memoria, int cuanto_X, char* buffer){
	char ** parametros_a_enviar = string_array_new();
	string_array_push(&parametros_a_enviar, string_itoa(dir_fisica_memoria));
	string_array_push(&parametros_a_enviar, string_itoa(cuanto_X));
	string_array_push(&parametros_a_enviar, buffer);
	enviar_msj_con_parametros(socket_kernel, ESCRIBIR, parametros_a_enviar); //TODO: no existe este msj entre el kernel y fs
	free(parametros_a_enviar);

	t_mensajes* mensaje = malloc(sizeof(t_mensajes));
	mensaje->cod_op = recibir_msj(socket_memoria);
	if(mensaje->cod_op != ESCRITO_OK)
		//kaboom??? TODO

	return;
}

//TODO debug
int cant_unos_en_bitmap(){
	int contador = 0;
	for (int i = 0; i < bitarray_get_max_bit(bitarray_de_bitmap); i++) {
		if (bitarray_test_bit(bitarray_de_bitmap, i) == 1) {
			contador++;
		}
	}
	return contador;
}

