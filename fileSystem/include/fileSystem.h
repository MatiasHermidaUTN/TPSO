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
#include "../include/configuracion_fileSystem.h"

extern pthread_mutex_t mutex_new_queue;
extern pthread_mutex_t mutex_ready_list;
//TODO extern de TODAS LAS VAR DE FS.c

typedef struct {
	char* nombre_archivo;
	int tamanio_archivo;
	uint32_t puntero_directo;
	uint32_t puntero_indirecto;
} t_fcb;

typedef enum
{
	ABRIR,
	CREAR,
	TRUNCAR,
	LEER,
	ESCRIBIR,
	ERROR,
} t_instrucciones;

struct super_bloque_info {
	int block_size;
	int block_count;
}super_bloque_info;

typedef struct args_recibir_mensajes {
	t_instrucciones cod_op;
	char* nombre_archivo;
	int nuevo_tamanio_archivo;
	int apartir_de_donde_X;
	int cuanto_X;
	int dir_fisica_memoria;
} t_mensajes;

//fcb
uint32_t config_get_uint_value(t_config *self, char *key);
char* obtener_path_FCB_sin_free(char* nombre_archivo);
//hilos
int recibir_mensajes();
int manejar_mensaje();
void escuchar_kernel();
//
uint32_t conseguir_ptr_secundario_para_indirecto(uint32_t puntero_indirecto, int nro_de_ptr_secundario);
void guardar_ptr_secundario_para_indirecto(uint32_t puntero_secundairo, uint32_t puntero_indirecto, int nro_de_ptr_secundario);
//bitmap
bool checkear_espacio(int cuanto_escribir);
uint32_t dame_un_bloque_libre();
void liberar_bloque(uint32_t puntero);
//abrir
bool existe_archivo(char* nombre_archivo);
bool archivo_se_puede_leer(char* path);
//crear
void crear_archivo(char* nombre_archivo);
//truncar
void truncar(char* nombre_archivo, int nuevo_tamanio_archivo);
void achicas_archivo(t_config* archivo_FCB, char* nombre_archivo, int nuevo_tamanio_archivo);
void agrandas_archivo(t_config* archivo_FCB, char* nombre_archivo, int nuevo_tamanio_archivo);
//leer
char* leer_archivo(char* nombre_archivo, int apartir_de_donde_leer, int cuanto_leer);
void leer_indirecto(char* buffer, t_config* archivo_FCB, int bloque_secundario_donde_leer, int* cuanto_leer, int* cantidad_leida);
void leer_bloque(char* buffer, uint32_t puntero, int apartir_de_donde_leer_relativo_a_bloque, int* cuanto_leer, int* cantidad_leida);
//escribir
void escribir_archivo(char* buffer, char* nombre_archivo, int apartir_de_donde_escribir, int cuanto_escribir);
void sobreescribir_indirecto(char* buffer, t_config* archivo_FCB, int bloque_secundario_donde_escribir, int* cuanto_escribir, int* cantidad_escrita);
void escribir_bloque(char* buffer, uint32_t puntero, int apartir_de_donde_escribir_relativo_a_puntero, int* cuanto_escribir, int* cantidad_escrita);

//comunicaciones
t_instrucciones recibir_cod_op(int socket_cliente);
void recibir_parametros(t_instrucciones cod_op, char** nombre_archivo, int* tamanio_nuevo_archivo, int* apartir_de_donde_X, int* cuanto_X, int* dir_fisica_memoria);
void deserializar_instrucciones_kernel(void* a_recibir, int size_payload, t_instrucciones cod_op, char** nombre_archivo, int* tamanio_nuevo_archivo, int* apartir_de_donde_X, int* cuanto_X, int* dir_fisica_memoria);
void enviar_mensaje_kernel(int socket_kernel, char* msj);
char* leer_de_memoria(int socket_memoria, t_instrucciones LEER, int cuanto_escribir, int dir_fisica_memoria);
void mandar_a_memoria(int socket_memoria, t_instrucciones ESCRIBIR, char* buffer, int cuanto_leer, int dir_fisica_memoria);

//debug
int cant_unos_en_bitmap();
void limpiar_bitmap();

#endif /* FILESYSTEM_H_ */
