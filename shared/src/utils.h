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

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>

typedef enum {
	MENSAJE,
	LIST_INSTRUCCIONES,
	PCB,
} op_code;

typedef enum {
	KERNEL,
	CPU,
	FILESYSTEM,
	OK,
	ERROR,
} t_handshake;

typedef enum {
	YIELD,
	IO,
	EXIT,
} t_rta_cpu_al_kernel;

typedef struct {
	int size;
	void* stream;
} t_buffer;

typedef struct {
	op_code codigo_operacion;
	t_buffer* buffer;
} t_paquete;


typedef struct {
	char* nombre;
	t_list* parametros;
} t_instruccion;

typedef struct registros_cpu {
	char  AX [4],  BX[ 4],  CX[ 4],  DX[ 4];
	char EAX [8], EBX[ 8], ECX[ 8], EDX[ 8];
	char RAX[16], RBX[16], RCX[16], RDX[16];
} t_registros_cpu;

typedef struct t_pcb {
	int pid;
	t_list* instrucciones;
	int pc;
	t_registros_cpu registros_cpu;
	t_list* tabla_segmentos;
	int estimado_prox_rafaga;
	int estimado_llegada_ready;
	t_list* archivos_abiertos;

	int socket_consola; //para mandarle mensaje que cuando termina
} t_pcb;

typedef struct args_recibir_conexiones {
	int socket_cliente;
	t_log* logger;
} t_args_recibir_conexiones;

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

#endif /* UTILS_H_ */
