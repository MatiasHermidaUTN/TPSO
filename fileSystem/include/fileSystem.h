#ifndef FILESYSTEM_H_
#define FILESYSTEM_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <commons/bitarray.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>
#include <utils.h>
#include "../include/configuracion_fileSystem.h"

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

bool archivo_se_puede_leer(char* path);
void crear_archivo(char* nombre_archivo);
bool existe_archivo(char* nombre_archivo);
char* obtener_path_FCB_sin_free(char* nombre_arhcivo);
char* leer_archivo(char* nombre_archivo, int apartir_de_donde_leer, int cuanto_leer);
//void leer_indirecto(char** buffer, uint32_t puntero_indirecto, int cuanto_leer);
void escribir_archivo(char* buffer, char* nombre_archivo, int apartir_de_donde_escribir, int cuanto_escribir);
bool archivo_se_puede_leer(char* path);
bool archivo_se_puede_escribir(char* path);
char* intToCharAsterisco(unsigned int numero);
int cantidad_de_digitos(unsigned int numero);
bool checkear_espacio(int cuanto_escribir);
uint32_t dame_un_bloque_libre();
//void escribir_bloque(char* buffer, uint32_t puntero, int* cuanto_escribir, int* cantidad_escrita, int espacio_libre_en_bloque);
int espacio_libre_en_bloque(uint32_t puntero);

//TODO
t_instrucciones recibir_cod_op(int socket_cliente);
void recibir_parametros(t_instrucciones cod_op, char** nombre_archivo, int* tamanio_nuevo_archivo, int* apartir_de_donde_X, int* cuanto_X, int* dir_fisica_memoria);
void deserializar_instrucciones_kernel(void* a_recibir, int size_payload, t_instrucciones cod_op, char** nombre_archivo, int* tamanio_nuevo_archivo, int* apartir_de_donde_X, int* cuanto_X, int* dir_fisica_memoria);
void enviar_mensaje_kernel(int socket_kernel, char* msj);
char* leer_de_memoria(int socket_memoria, t_instrucciones LEER, int cuanto_escribir, int dir_fisica_memoria);
void mandar_a_memoria(int socket_memoria, t_instrucciones ESCRIBIR, char* buffer, int cuanto_leer, int dir_fisica_memoria);
void liberar_bloque(uint32_t puntero);

#endif /* FILESYSTEM_H_ */
