/////////////////////////
// Utils.c del cliente //
/////////////////////////

#ifndef UTILS_H_
#define UTILS_H_

#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<unistd.h>
#include<sys/socket.h>
#include<netdb.h>
#include<string.h>
#include<commons/log.h>
#include<commons/config.h>
#include<commons/collections/queue.h>
#include<semaphore.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <pthread.h>


typedef enum
{
	MENSAJE,
} op_code;

typedef enum
{
	KERNEL,
	CPU,
	FILESYSTEM,
	OK_HANDSHAKE,
	ERROR_HANDSHAKE,
} t_handshake;

typedef enum
{
	YIELD_EJECUTADO,
	IO_EJECUTADO,
	EXIT_EJECUTADO,
	PCB_A_EJECUTAR,
}t_msj_kernel_cpu;

typedef enum
{
	LIST_INSTRUCCIONES,
	FINALIZACION_OK,
}t_msj_kernel_consola;

typedef enum
{
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
}t_enum_instruccion;


typedef struct
{
	int size;
	void* stream;
} t_buffer;

typedef struct
{
	op_code codigo_operacion;
	t_buffer* buffer;
} t_paquete;

typedef struct registros_cpu{
	char AX[4],BX[4],CX[4],DX[4];
	char EAX[8],EBX[8],ECX[8],EDX[8];
	char RAX[16],RBX[16],RCX[16],RDX[16];
}t_registros_cpu;

typedef struct pcb{
	int pid;
	t_list* instrucciones;
	int pc;
	t_registros_cpu registros_cpu;
	t_list* tabla_segmentos;
	int estimado_prox_rafaga;
	int estimado_llegada_ready;
	t_list* archivos_abiertos;
	int socket_consola; //para mandarle mensaje que cuando termina

}t_pcb;

typedef struct {
	char* nombre;
	t_list* parametros;
} t_instruccion;


/*
int crear_conexion(char* ip, char* puerto);
void enviar_mensaje(char* mensaje, int socket_cliente);
t_paquete* crear_paquete(void);
t_paquete* crear_super_paquete(void);
void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio);
void enviar_paquete(t_paquete* paquete, int socket_cliente);
void liberar_conexion(int socket_cliente);
void eliminar_paquete(t_paquete* paquete);

//////////////////////////
// Utils.c del servidor //
//////////////////////////

t_log* iniciar_logger(void);
t_config* iniciar_config(void);
void leer_consola(t_log*);
void paquete(int);
void terminar_programa(int, t_log*, t_config*);*/


int crear_conexion(char *ip, char* puerto);

void enviar_mensaje(char* mensaje, int socket_cliente);

int iniciar_servidor(char* IP, char* PUERTO);

int esperar_cliente(int socket_servidor);

int recibir_operacion(int socket_cliente);

void recibir_mensaje(int socket_cliente);

void* serializar_paquete(t_paquete* paquete, int bytes);

void* recibir_buffer(int* size, int socket_cliente);

void eliminar_paquete(t_paquete* paquete);

t_config* iniciar_config(char* path);

t_log* iniciar_logger(char* path,char* nombre);

t_handshake recibir_handshake(int socket_cliente);

void enviar_handshake(int socket, t_handshake t_handshake);

t_msj_kernel_consola recibir_fin_proceso(int socket_cliente);

void enviar_fin_proceso(int socket, t_msj_kernel_consola msj);

void *queue_pop_con_mutex(t_queue* queue, pthread_mutex_t* mutex);

void queue_push_con_mutex(t_queue* queue,void* elemento , pthread_mutex_t* mutex);

void *list_pop_con_mutex(t_list* lista, pthread_mutex_t* mutex);

void list_push_con_mutex(t_list* lista,void* elemento , pthread_mutex_t* mutex);

char* obtener_pids(t_list* lista_pcbs);

#endif /* UTILS_H_ */
