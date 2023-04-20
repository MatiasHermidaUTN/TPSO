#include "../include/ejecucion_instrucciones.h"

void ejecutar_instrucciones(t_pcb* pcb){
	int cant_instrucciones = list_size(pcb->instrucciones);
	t_instruccion* instruccion_actual;
	while(pcb->pc < cant_instrucciones){
		//Fetch
		instruccion_actual = list_get(pcb->instrucciones, pcb->pc);
		pcb->pc ++;
		//Decode y Execute
		switch(instruccion_a_enum(instruccion_actual)){
			case SET:
				ejecutar_set(pcb, instruccion_actual);
				break;
			case MOV_IN:
				break;
			case MOV_OUT:
				break;
			case IO:
				break;
			case F_OPEN:
				break;
			case F_CLOSE:
				break;
			case F_SEEK:
				break;
			case F_READ:
				break;
			case F_WRITE:
				break;
			case F_TRUNCATE:
				break;
			case WAIT:
				break;
			case SIGNAL:
				break;
			case CREATE_SEGMENT:
				break;
			case DELETE_SEGMENT:
				break;
			case YIELD:
				enviar_pcb(socket_kernel, pcb, YIELD_EJECUTADO);
				return;
				break;
			case EXIT:
				enviar_pcb(socket_kernel, pcb, EXIT);
				return;
				break;
			default:
				break;
		}
	}
}

t_enum_instruccion instruccion_a_enum(t_instruccion* instruccion){
	char* nombre = instruccion->nombre;
	if(!strcmp(nombre, "SET")) return SET;
	if(!strcmp(nombre, "MOV_IN")) return MOV_IN;
	if(!strcmp(nombre, "MOV_OUT")) return MOV_OUT;
	if(!strcmp(nombre, "I/O")) return IO;
	if(!strcmp(nombre, "F_OPEN")) return F_OPEN;
	if(!strcmp(nombre, "F_CLOSE")) return F_CLOSE;
	if(!strcmp(nombre, "F_SEEK")) return F_SEEK;
	if(!strcmp(nombre, "F_READ")) return F_READ;
	if(!strcmp(nombre, "F_WRITE")) return F_WRITE;
	if(!strcmp(nombre, "F_TRUNCATE")) return F_TRUNCATE;
	if(!strcmp(nombre, "WAIT")) return WAIT;
	if(!strcmp(nombre, "SINGAL")) return SIGNAL;
	if(!strcmp(nombre, "CREATE_SEGMEN")) return CREATE_SEGMENT;
	if(!strcmp(nombre, "DELETE_SEGMENT")) return DELETE_SEGMENT;
	if(!strcmp(nombre, "YIELD")) return YIELD;
	if(!strcmp(nombre, "EXIT")) return EXIT;
	return INSTRUCCION_ERRONEA;
}

void ejecutar_set(t_pcb* pcb, t_instruccion* instruccion_actual){
	char* registro = list_get(instruccion_actual->parametros, 0);
	char* valor = list_get(instruccion_actual->parametros, 1);

	if(!strcmp(registro, "AX")){
		memcpy(pcb->registros_cpu.AX, valor, 4);
	}
	else if(!strcmp(registro, "BX")){
		memcpy(pcb->registros_cpu.BX, valor, 4);
	}
	else if(!strcmp(registro, "CX")){
		memcpy(pcb->registros_cpu.CX, valor, 4);
	}
	else if(!strcmp(registro, "DX")){
		memcpy(pcb->registros_cpu.DX, valor, 4);
	}
	else if(!strcmp(registro, "EAX")){
		memcpy(pcb->registros_cpu.EAX, valor, 8);
	}
	else if(!strcmp(registro, "EBX")){
		memcpy(pcb->registros_cpu.EBX, valor, 8);
	}
	else if(!strcmp(registro, "ECX")){
		memcpy(pcb->registros_cpu.ECX, valor, 8);
	}
	else if(!strcmp(registro, "EDX")){
		memcpy(pcb->registros_cpu.EDX, valor, 8);
	}
	else if(!strcmp(registro, "RAX")){
		memcpy(pcb->registros_cpu.RAX, valor, 16);
	}
	else if(!strcmp(registro, "RBX")){
		memcpy(pcb->registros_cpu.RBX, valor, 16);
	}
	else if(!strcmp(registro, "RCX")){
		memcpy(pcb->registros_cpu.RCX, valor, 16);
	}
	else if(!strcmp(registro, "RDX")){
		memcpy(pcb->registros_cpu.RDX, valor, 16);
	}
	sleep(atoi(lectura_de_config.RETARDO_INSTRUCCION));
	return;
}


