#ifndef FILESYSTEM_H_
#define FILESYSTEM_H_

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

#include "configuracion_fileSystem.h"
#include "manejar_mensajes.h"
#include "conexiones_fileSystem.h"

#define PRIMER_BLOQUE_SECUNDARIO 1
#define DESDE_EL_INICIO 0

typedef struct {
	char* nombre_archivo;
	int tamanio_archivo;
	uint32_t puntero_directo;
	uint32_t puntero_indirecto;
} t_fcb;

struct super_bloque_info {
	int block_size;
	int block_count;
};

extern struct super_bloque_info super_bloque_info;

extern pthread_mutex_t mutex_new_queue;
extern pthread_mutex_t mutex_ready_list;

extern t_config* superbloque;
extern void* bitmap;
extern void* bitmap_pointer;
extern t_bitarray* bitarray_de_bitmap;
extern FILE* bloques;
extern int tamanioBitmap;

extern t_log* logger;
extern t_log* my_logger;
extern t_config* config;

extern int kernel;
extern int socket_memoria;

extern pthread_mutex_t mutex_cola_msj;
extern sem_t sem_sincro_cant_msj;

extern t_list* lista_fifo_msj;

//comunicaciones
char* leer_de_memoria(int cuanto_X, int dir_fisica_memoria, char*pid);
void escribir_en_memoria(int dir_fisica_memoria, int cuanto_leer, char* buffer, char*pid);

//debug
int cant_unos_en_bitmap();

#endif /* FILESYSTEM_H_ */
