#ifndef UTILS_H_
#define UTILS_H_

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <math.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <commons/string.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/queue.h>
#include <semaphore.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <pthread.h>
#include "diccionario_instrucciones.h"

///////////
// ENUMS //
///////////

typedef enum {
	KERNEL,
	CPU,
	FILESYSTEM,

	OK_HANDSHAKE,
	ERROR_HANDSHAKE,
} t_handshake;

typedef enum {
	//Las que usa solo CPU:
	PCB_A_EJECUTAR,

	INSTRUCCION_ERRONEA,

	SET,
	MOV_IN,
	MOV_OUT,

	//Las que usan CPU y Memoria:
	IO,
	F_OPEN,
	F_CLOSE,
	F_SEEK,
	F_READ,
	F_WRITE,
	F_TRUNCATE,
	WAIT,
	SIGNAL,
	CREATE_SEGMENT,
	DELETE_SEGMENT,
	YIELD,
	EXIT,

	//Las que usa solo Memoria
	EXIT_CON_SEG_FAULT,
} t_msj_kernel_cpu;

typedef enum {
	//La que manda Consola:
	LIST_INSTRUCCIONES,

	//Las que manda Kernel:
	SUCCESS,
	OUT_OF_MEMORY,
	SEG_FAULT,
	RECURSO_INEXISTENTE,
} t_msj_kernel_consola;

typedef enum {
	//Las que manda Kernel:
	EXISTE_ARCHIVO,
	CREAR_ARCHIVO,
	TRUNCAR_ARCHIVO,
	LEER_ARCHIVO,
	ESCRIBIR_ARCHIVO,

	//Las que manda FS:
	EL_ARCHIVO_YA_EXISTE,
	EL_ARCHIVO_NO_EXISTE,
	EL_ARCHIVO_FUE_CREADO,
	EL_ARCHIVO_FUE_TRUNCADO,
	EL_ARCHIVO_FUE_LEIDO,
	EL_ARCHIVO_FUE_ESCRITO,
} t_msj_kernel_fileSystem;

typedef enum {
	//Las que manda Kernel:
	CREAR_SEGMENTO,
	ELIMINAR_SEGMENTO,
	INICIALIZAR_PROCESO,
	ELIMINAR_PROCESO,
	COMPACTAR,

	//Las que manda CPU:
	LEER_VALOR,
	ESCRIBIR_VALOR,
//TODO: fijarse si las 2 de arriba son necesarias o CPU y FS las pueden usar por igual
	//Las que manda FS
	LEER,
	ESCRIBIR,

	//Las que manda Memoria
	SEGMENTO_CREADO,
	NO_HAY_ESPACIO_DISPONIBLE,
	HAY_QUE_COMPACTAR,
	SEGMENTO_ELIMINADO,	
	PROCESO_INICIALIZADO,
	PROCESO_ELIMINADO,	
	MEMORIA_COMPACTADA,

	ESCRITO_OK,
	LEIDO_OK,
} t_msj_memoria;

//////////////
// STRUCTS //
//////////////

typedef struct registros_cpu {
	char  AX[ 4],  BX[ 4],  CX[ 4],  DX[ 4];
	char EAX[ 8], EBX[ 8], ECX[ 8], EDX[ 8];
	char RAX[16], RBX[16], RCX[16], RDX[16];
} t_registros_cpu;

typedef struct pcb {
	int pid;
	t_list* instrucciones;
	int pc;
	t_registros_cpu registros_cpu;
	t_list* tabla_segmentos;
	double estimado_prox_rafaga;
	int tiempo_llegada_ready;
	t_list* archivos_abiertos;

	t_list* recursos;

	int socket_consola; //para mandarle mensaje que cuando termina

	double tiempo_real_ejecucion;
	double tiempo_inicial_ejecucion;
} t_pcb;

typedef struct {
	char* nombre;
	t_list* parametros;
} t_instruccion;

typedef struct archivo_abierto {
	char* nombre_archivo;
	int posicion_actual;
} t_archivo_abierto;

typedef struct segmento {
	int id;
	int direccion_base;
	int tamanio;
} t_segmento;

typedef struct {
	int pid;
	t_list* tabla_segmentos;
} t_proceso_actualizado;

/////////////
// Cliente //
/////////////

int crear_conexion(char* ip, char* puerto);

//////////////
// Servidor //
//////////////

int iniciar_servidor(char* IP, char* PUERTO);

int esperar_cliente(int socket_servidor);

t_config* iniciar_config(char* path);
t_log* iniciar_logger(char* path, char* nombre);

t_handshake recibir_handshake(int socket_cliente);
void enviar_handshake(int socket, t_handshake t_handshake);

//////////////////////
// Listas de estado //
//////////////////////

void queue_push_con_mutex(t_queue* queue, void* elemento, pthread_mutex_t* mutex);
void* queue_pop_con_mutex(t_queue* queue, pthread_mutex_t* mutex);
void list_push_con_mutex(t_list* lista, void* elemento, pthread_mutex_t* mutex);
void* list_pop_con_mutex(t_list* lista, pthread_mutex_t* mutex);

///////////////////
// Liberar datos //
///////////////////

void liberar_pcb(t_pcb* pcb);
void destruir_instruccion(t_instruccion* instruccion);
void destruir_archivo_abierto(t_archivo_abierto* archivo);
void liberar_tabla_segmentos(t_list* tabla_segmentos);
void liberar_parametros(char** parametros);

/////////
// pcb //
/////////

void enviar_pcb(int socket, t_pcb* pcb, t_msj_kernel_cpu op_code, char** parametros_de_instruccion);

void* serializar_pcb(t_pcb* pcb, size_t* size_total, t_msj_kernel_cpu op_code, char** parametros_de_instruccion);
size_t tamanio_payload_pcb(t_pcb* pcb);
size_t tamanio_instrucciones(t_list* instrucciones);
int tamanio_parametros(t_list* parametros, int index_instruccion);

void memcpy_instrucciones_serializar(void* stream_pcb, t_list* instrucciones, int* desplazamiento);
void memcpy_registros_serializar(void* stream_pcb, t_registros_cpu registros_cpu, int* desplazamiento);
void memcpy_tabla_segmentos_serializar(void* stream, t_list* tabla_segmentos, int* desplazamiento);
void memcpy_archivos_abiertos_serializar(void* stream, t_list* archivos_abiertos, int* desplazamiento);
void memcpy_recursos_serializar(void* stream, t_list* recursos, int* desplazamiento);

t_pcb* recibir_pcb(int socket);

t_pcb* deserializar_pcb(void* stream);
t_list* deserializar_instrucciones(void* a_recibir, size_t size_payload, int* desplazamiento);
void deserializar_parametros(void* a_recibir, int* desplazamiento, t_instruccion* instruccion);

void memcpy_registros_deserializar(t_registros_cpu* registros_cpu, void* stream_pcb, int* desplazamiento);
void memcpy_tabla_segmentos_deserializar(t_pcb* pcb, void* stream, int* desplazamiento);
void memcpy_archivos_abiertos_deserializar(t_pcb* pcb, void* stream, int* desplazamiento);
void memcpy_recursos_deserializar(t_pcb* pcb, void* stream, int* desplazamiento);

void print_l_instrucciones(t_list* instrucciones);

//////////////
// Mensajes //
//////////////

void enviar_msj(int socket, int msj);
int recibir_msj(int socket);
void enviar_msj_con_parametros(int socket, int op_code, char** parametros);
char** recibir_parametros_de_mensaje(int socket);

///////////////////////////
// Segmentos actualizdos //
///////////////////////////

void enviar_tabla_segmentos(int socket, t_list* tabla_segmentos, t_msj_memoria mensaje);
void* serializar_tabla_segmentos(t_list* tabla_segmentos, t_msj_memoria mensaje, size_t* size_total);
t_list* recibir_tabla_segmentos(int socket);
t_list* deserializar_tabla_segmentos(void* stream, int size_payload);

void enviar_procesos_con_segmentos(int socket, t_list* procesos_actualizados);
void* serializar_procesos_con_segmentos(t_list* procesos_actualizados, size_t* size_total);
t_list* recibir_procesos_con_segmentos(int socket);
t_list* deserializar_procesos_con_segmentos(void* stream, size_t size_payload);
#endif /* UTILS_H_ */
