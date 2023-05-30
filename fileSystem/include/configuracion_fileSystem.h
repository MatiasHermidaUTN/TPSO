#ifndef CONFIGURACION_FILESYSTEM_H_
#define CONFIGURACION_FILESYSTEM_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include <math.h>
#include <time.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <commons/bitarray.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>
#include <utils.h>

#include "fileSystem.h"
#include "arbir_crear_archivo.h"
#include "manejar_mensajes.h"

typedef struct fileSystem_config {
	char* IP_MEMORIA;
	char* PUERTO_MEMORIA;
	char* PUERTO_ESCUCHA;
	char* PATH_SUPERBLOQUE;
	char* PATH_BITMAP;
	char* PATH_BLOQUES;
	char* PATH_FCB;
	char* RETARDO_ACCESO_BLOQUE;
} t_fileSystem_config;

typedef struct {
	char* nombre_archivo;
	int tamanio_archivo;
	uint32_t puntero_directo;
	uint32_t puntero_indirecto;
} t_fcb;

typedef struct super_bloque_info {
	int block_size;
	int block_count;
} t_super_bloque_info;

extern pthread_mutex_t mutex_new_queue;
extern pthread_mutex_t mutex_ready_list;

extern t_config* superbloque;
extern void* bitmap;
extern void* bitmap_pointer;
extern t_bitarray* bitarray_de_bitmap;
extern FILE* bloques;
extern int tamanioBitmap;

extern t_log* logger;
extern t_config* config;
extern t_fileSystem_config lectura_de_config;

extern int socket_kernel;
extern int socket_memoria;

extern pthread_mutex_t mutex_cola_msj;
extern sem_t sem_sincro_cant_msj;

extern t_list* lista_fifo_msj;

extern t_super_bloque_info super_bloque_info;

t_fileSystem_config leer_fileSystem_config(t_config* config);

void init_superbloque();
void init_bitmap();
void init_bloques();
void init_carpeta_fcbs();

//comunicaciones
char* leer_de_memoria(int cuanto_X, int dir_fisica_memoria);
void escribir_en_memoria(int dir_fisica_memoria, int cuanto_X, char* buffer);

//debug
int cant_unos_en_bitmap();

#endif /* CONFIGURACION_FILESYSTEM_H_ */
