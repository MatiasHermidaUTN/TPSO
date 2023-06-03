#ifndef EJECUCION_INSTRUCCIONES_H_
#define EJECUCION_INSTRUCCIONES_H_

#include <string.h>
#include <utils.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <commons/string.h>
#include <pthread.h>
#include "configuracion_cpu.h"

typedef struct {
	int numero_segmento;
	int desplazamiento_segmento;
	int tamanio_segmento;
	int direccion_fisica;
} t_datos_mmu;

void ejecutar_instrucciones(t_pcb* pcb);

t_msj_kernel_cpu instruccion_a_enum(t_instruccion* instruccion);

void set_registro(t_pcb* pcb, char* registro, char* valor);
char* leer_registro(t_pcb* pcb, char* registro);
int tamanio_registro(char* nombre_registro);

void enviar_pcb_a_kernel(t_pcb* pcb, t_msj_kernel_cpu mensaje, t_list* list_parametros);

char* obtener_parametros_a_emitir(t_list* parametros_actuales);

int buscar_campo_de_segmento(t_list* segmentos, char* campo, int id);

t_datos_mmu mmu(t_pcb* pcb, int direccion_logica);


// Funciones de abstracción de lógica

int numero_de_parametro_de_direccion_logica(t_msj_kernel_cpu instruccion_actual);

int obtener_cantidad_de_bytes(t_msj_kernel_cpu instruccion_actual, t_list* parametros);

char* obtener_registro(t_msj_kernel_cpu instruccion_actual, t_list* parametros);

int numero_de_parametro_de_registro(t_msj_kernel_cpu instruccion_actual);


//Funciones de ejecución posta

void ejecutar_mov_in(t_pcb* pcb, t_datos_mmu datos_mmu, char* nombre_registro);

void ejecutar_mov_out(t_pcb* pcb, t_datos_mmu datos_mmu, char* nombre_registro);

void ejecutar_fread_o_fwrite(t_pcb* pcb, int direccion_fisica, t_msj_kernel_cpu instruccion_actual, t_list* parametros);

#endif /* EJECUCION_INSTRUCCIONES_H_ */



