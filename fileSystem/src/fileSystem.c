#include "../include/fileSystem.h"

struct super_bloque_info super_bloque_info;

t_config* superbloque;
void* bitmap;
void* bitmap_pointer;
t_bitarray* bitarray_de_bitmap;
FILE* bloques;
int tamanioBitmap;

t_log* logger;
t_config* config;
t_fileSystem_config lectura_de_config;

int kernel;
int socket_memoria;

pthread_mutex_t mutex_cola_msj;
sem_t sem_sincro_cant_msj;

t_list* lista_fifo_msj;

int main(int argc, char** argv) {

	//LECTURA DE CONFIG DEL FILESYSTEM
	logger = iniciar_logger("FileSystem.log", "FS");
	config = iniciar_config("../fileSystem.config");
    lectura_de_config = leer_fileSystem_config(config);

	lista_fifo_msj = list_create();

	sem_init(&sem_sincro_cant_msj, 0, 0); 		//msj es tarea a ejecutar(escribir, leer, etc.)
	pthread_mutex_init(&mutex_cola_msj, NULL);

	//SUPERBLOQUE, BITMAP, BLOQUES, carpeta FCBs
	init_superbloque();
	init_bitmap();
	init_bloques();
	init_carpeta_fcbs();

	init_conexiones();

	while(manejar_mensaje());

	log_destroy(logger);
    config_destroy(config);
	config_destroy(superbloque);
	msync(bitmap_pointer, tamanioBitmap, MS_SYNC);
	munmap(bitmap_pointer, tamanioBitmap);
	fclose(bloques);
	return EXIT_SUCCESS;
}

//comunicaciones
char* leer_de_memoria(int dir_fisica_memoria, int cuanto_X){
	char ** parametros_a_enviar = string_array_new();

	char* s_aux = string_itoa(dir_fisica_memoria);
	string_array_push(&parametros_a_enviar, s_aux);

	char* s_aux2 = string_itoa(cuanto_X);
	string_array_push(&parametros_a_enviar, s_aux2);

	enviar_msj_con_parametros(socket_memoria, LEER_VALOR, parametros_a_enviar);

	char* buffer = malloc(cuanto_X);
	t_mensajes* mensaje = malloc(sizeof(t_mensajes));

	mensaje->cod_op = recibir_msj(socket_memoria);


	if(mensaje->cod_op != LEIDO_OK){
		//kaboom??? TODO
	}
		
	mensaje->parametros = recibir_parametros_de_mensaje(socket_memoria);

	memcpy(buffer, mensaje->parametros[0], cuanto_X);
	string_array_destroy(parametros_a_enviar);
	string_array_destroy(mensaje->parametros);
	free(mensaje);

	return buffer;
}

void escribir_en_memoria(int dir_fisica_memoria, int cuanto_X, char* buffer){
	char** parametros_a_enviar = string_array_new();
	
	char* s_aux = string_itoa(dir_fisica_memoria);
	string_array_push(&parametros_a_enviar, s_aux);

	memcpy(buffer + cuanto_X, "\0", 1); //agrega el /0 al final del buffer

	string_array_push(&parametros_a_enviar, buffer);
	enviar_msj_con_parametros(socket_memoria, ESCRIBIR_VALOR, parametros_a_enviar);
	string_array_destroy(parametros_a_enviar);

	t_mensajes* mensaje = malloc(sizeof(t_mensajes));
	mensaje->cod_op = recibir_msj(socket_memoria);
	if(mensaje->cod_op != ESCRITO_OK){
		//kaboom??? TODO
	}
	free(mensaje);
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
