/////////////////////////
// Utils.c del cliente //
/////////////////////////

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

typedef enum {
	MENSAJE,
} op_code;

typedef enum {
	KERNEL,
	CPU,
	FILESYSTEM,

	OK_HANDSHAKE,
	ERROR_HANDSHAKE,
} t_handshake;

typedef enum {
	PCB_A_EJECUTAR,

	IO_EJECUTADO,
	MOV_IN_EJECUTADO,
	MOV_OUT_EJECUTADO,
	F_OPEN_EJECUTADO,
	F_CLOSE_EJECUTADO,
	F_SEEK_EJECUTADO,
	F_READ_EJECUTADO,
	F_WRITE_EJECUTADO,
	F_TRUNCATE_EJECUTADO,
	WAIT_EJECUTADO,
	SIGNAL_EJECUTADO,
	CREATE_SEGMENT_EJECUTADO,
	DELETE_SEGMENT_EJECUTADO,
	YIELD_EJECUTADO,
	EXIT_EJECUTADO,

	EXIT_CON_SEG_FAULT_EJECUTADO,
} t_msj_kernel_cpu;

typedef enum {
	LIST_INSTRUCCIONES,
	SUCCESS,
	OUT_OF_MEMORY,
	SEG_FAULT,
} t_msj_kernel_consola;

typedef enum {
	SET,
	MOV_IN,
	MOV_OUT,
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

	INSTRUCCION_ERRONEA,
} t_enum_instruccion;

typedef enum {
	EXISTE_ARCHIVO,
	CREAR_ARCHIVO,
	TRUNCAR_ARCHIVO,
	LEER_ARCHIVO,
	ESCRIBIR_ARCHIVO,

	EL_ARCHIVO_YA_EXISTE,
	EL_ARCHIVO_NO_EXISTE,
	EL_ARCHIVO_FUE_CREADO,
	EL_ARCHIVO_FUE_TRUNCADO,
	EL_ARCHIVO_FUE_LEIDO,
	EL_ARCHIVO_FUE_ESCRITO,
} t_msj_kernel_fileSystem;

typedef enum {
	CREAR_SEGMENTO,
	ELIMINAR_SEGMENTO,
	INICIALIZAR_PROCESO,
	ELIMINAR_PROCESO,
	COMPACTAR,

	SEGMENTO_CREADO,
	NO_HAY_ESPACIO_DISPONIBLE,
	HAY_QUE_COMPACTAR,
	SEGMENTO_ELIMINADO,
	PROCESO_INICIALIZADO,
	PROCESO_ELIMINADO,
	MEMORIA_COMPACTADA,
} t_msj_kernel_memoria;

typedef enum {
	LEER_VALOR,
	ESCRIBIR_VALOR,

	ESCRITO_OK,
	LEIDO_OK,
} t_msj_cpu_memoria;

typedef struct {
	int size;
	void* stream;
} t_buffer;

typedef struct {
	op_code codigo_operacion;
	t_buffer* buffer;
} t_paquete;

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
	double tiempo_llegada_ready;
	t_list* archivos_abiertos;

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

int crear_conexion(char *ip, char* puerto);

void enviar_mensaje(char* mensaje, int socket_cliente);

//////////////
// Servidor //
//////////////

int iniciar_servidor(char* IP, char* PUERTO);

int esperar_cliente(int socket_servidor);

int recibir_operacion(int socket_cliente);
void recibir_mensaje(int socket_cliente);

void* serializar_paquete(t_paquete* paquete, int bytes);
void* recibir_buffer(int* size, int socket_cliente);
void eliminar_paquete(t_paquete* paquete);

t_config* iniciar_config(char* path);
t_log* iniciar_logger(char* path, char* nombre);

t_handshake recibir_handshake(int socket_cliente);
void enviar_handshake(int socket, t_handshake t_handshake);

t_msj_kernel_consola recibir_fin_proceso(int socket_cliente);
void enviar_fin_proceso(int socket, t_msj_kernel_consola msj);

//////////////////////
// Listas de estado //
//////////////////////

void *queue_pop_con_mutex(t_queue* queue, pthread_mutex_t* mutex);
void queue_push_con_mutex(t_queue* queue, void* elemento, pthread_mutex_t* mutex);
void *list_pop_con_mutex(t_list* lista, pthread_mutex_t* mutex);
void list_push_con_mutex(t_list* lista, void* elemento, pthread_mutex_t* mutex);

///////////////////
// Liberar datos //
///////////////////

void destruir_instruccion(t_instruccion* instruccion);
void liberar_pcb(t_pcb* pcb);
void liberar_parametros(char** parametros);
void destruir_archivo_abierto(t_archivo_abierto* archivo);
void liberar_tabla_segmentos(t_list* tabla_segmentos);

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

t_pcb* recibir_pcb(int socket);

t_pcb* deserializar_pcb(void* stream);
t_list* deserializar_instrucciones(void* a_recibir, size_t size_payload, int* desplazamiento);
void deserializar_parametros(void* a_recibir, int* desplazamiento, t_instruccion* instruccion, t_dictionary* diccionario_instrucciones);

void memcpy_registros_deserializar(t_registros_cpu* registros_cpu, void* stream_pcb, int* desplazamiento);
void memcpy_tabla_segmentos_deserializar(t_pcb* pcb, void* stream, int* desplazamiento);
void memcpy_archivos_abiertos_deserializar(t_pcb* pcb, void* stream, int* desplazamiento);

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

void enviar_tabla_segmentos(int socket, t_list* tabla_segmentos);
void* serializar_tabla_segmentos(t_list* tabla_segmentos, size_t* size_total);
t_list* recibir_tabla_segmentos(int socket);
t_list* deserializar_tabla_segmentos(void* stream);

void enviar_procesos_con_segmentos(int socket, t_list* procesos_actualizados);
void* serializar_procesos_con_segmentos(t_list* procesos_actualizados, size_t* size_total);
t_list* recibir_procesos_con_segmentos(int socket);
t_list* deserializar_procesos_con_segmentos(void* stream);

#endif /* UTILS_H_ */
