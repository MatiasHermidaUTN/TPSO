#ifndef PCB_KERNEL_H_
#define PCB_KERNEL_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <commons/collections/list.h>
#include <commons/string.h>

typedef struct registros_cpu { // Va en el shared?
	char*  AX,  BX,  CX,  DX; //4 bytes
	char* EAX, EBX, ECX, EDX; //8 bytes
	char* RAX, RBX, RCX, RDX; //16 bytes
} t_registros_cpu;

typedef struct segmento {
	int id;
	char* direccion_base;
	int tamanio;
} t_segmento;

typedef struct archivo {
	char* nombre;
	void* puntero;
} t_archivo;

typedef struct pcb {
	int pid;
	t_list* instrucciones; //del tipo t_instruccion
	int pc;
	t_registros_cpu* registros_cpu; // Tiene que ser puntero?
	t_list* tabla_de_segmentos; //del tipo t_segmento
	int estimado_de_proxima_rafaga;
	int tiempo_de_llegada_a_ready;
	t_list* archivos_abiertos; //del tipo t_archivo
} t_pcb;

t_pcb* generar_pcb(t_list* proceso, int config_estimado_de_proxima_rafaga);

int generar_pid();

t_registros_cpu* generar_registros_cpu();

#endif /* PCB_KERNEL_H_ */
