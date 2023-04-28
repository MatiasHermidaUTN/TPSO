#include "../include/ejecucion_instrucciones.h"

t_log* logger;

void ejecutar_instrucciones(t_pcb* pcb) {
	if(!pcb->tiempo_inicial_ejecucion) { //Si no viene de EXEC
		pcb->tiempo_inicial_ejecucion = time(NULL); // Para calcular el tiempo en ejecucion
	}

	int cant_instrucciones = list_size(pcb->instrucciones);
	t_instruccion* instruccion_actual;
	while(pcb->pc < cant_instrucciones) {
		//Fetch
		instruccion_actual = list_get(pcb->instrucciones, pcb->pc);
		pcb->pc++;
		log_info(logger, "PID: %d - Ejecutando: %s - <PARAMETROS>", pcb->pid, instruccion_actual->nombre); //log obligatorio
		//TODO: emitir los parametros
		//Decode y Execute
		switch(instruccion_a_enum(instruccion_actual)) {
			case SET:
				ejecutar_set(pcb, instruccion_actual);
				break;
			case MOV_IN:
				break;
			case MOV_OUT:
				break;
			case IO:
				pcb->tiempo_real_ejecucion = time(NULL) - pcb->tiempo_inicial_ejecucion;
				pcb->tiempo_inicial_ejecucion = 0;

				char* tiempo_a_bloquear = list_get(instruccion_actual->parametros, 0); //va con strdup?
				//printf("El tiempo a bloquear de %d es: %s.\n", pcb->pid, tiempo_a_bloquear);
				enviar_pcb(socket_kernel, pcb, IO_EJECUTADO, tiempo_a_bloquear);
				return;
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
				//No se actualiza el tiempo_real_ejecucion, ya que se considera que sigue en EXEC (Running)
				char* recurso_a_usar = list_get(instruccion_actual->parametros, 0); //va con strdup?
				enviar_pcb(socket_kernel, pcb, WAIT_EJECUTADO, recurso_a_usar);
				log_warning(logger, "Se leyo un WAIT\n");
				return;
				break;
			case SIGNAL:
				//No se actualiza el tiempo_real_ejecucion, ya que se considera que sigue en EXEC (Running)
				char* recurso_a_desbloquear = list_get(instruccion_actual->parametros, 0); //va con strdup?
				enviar_pcb(socket_kernel, pcb, SIGNAL_EJECUTADO, recurso_a_desbloquear);
				log_warning(logger, "Se leyo un SIGNAL\n");
				return;
				break;
			case CREATE_SEGMENT:
				break;
			case DELETE_SEGMENT:
				break;
			case YIELD:
				pcb->tiempo_real_ejecucion = time(NULL) - pcb->tiempo_inicial_ejecucion;
				pcb->tiempo_inicial_ejecucion = 0;
				//printf("tiempo_real_ejecucion de %d: %d.\n", pcb->pid, pcb->tiempo_real_ejecucion);
				enviar_pcb(socket_kernel, pcb, YIELD_EJECUTADO, NULL); //NULL porque no se le pasa ningun parametro
				return;
				break;
			case EXIT:
				enviar_pcb(socket_kernel, pcb, EXIT_EJECUTADO, NULL); //NULL porque no se le pasa ningun parametro
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
	if(!strcmp(nombre, "SIGNAL")) return SIGNAL;
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
		memcpy(pcb->registros_cpu.AX, valor, 4*sizeof(char));
	}
	else if(!strcmp(registro, "BX")){
		memcpy(pcb->registros_cpu.BX, valor, 4*sizeof(char));
	}
	else if(!strcmp(registro, "CX")){
		memcpy(pcb->registros_cpu.CX, valor, 4*sizeof(char));
	}
	else if(!strcmp(registro, "DX")){
		memcpy(pcb->registros_cpu.DX, valor, 4*sizeof(char));
	}
	else if(!strcmp(registro, "EAX")){
		memcpy(pcb->registros_cpu.EAX, valor, 8*sizeof(char));
	}
	else if(!strcmp(registro, "EBX")){
		memcpy(pcb->registros_cpu.EBX, valor, 8*sizeof(char));
	}
	else if(!strcmp(registro, "ECX")){
		memcpy(pcb->registros_cpu.ECX, valor, 8*sizeof(char));
	}
	else if(!strcmp(registro, "EDX")){
		memcpy(pcb->registros_cpu.EDX, valor, 8*sizeof(char));
	}
	else if(!strcmp(registro, "RAX")){
		memcpy(pcb->registros_cpu.RAX, valor, 16*sizeof(char));
	}
	else if(!strcmp(registro, "RBX")){
		memcpy(pcb->registros_cpu.RBX, valor, 16*sizeof(char));
	}
	else if(!strcmp(registro, "RCX")){
		memcpy(pcb->registros_cpu.RCX, valor, 16*sizeof(char));
	}
	else if(!strcmp(registro, "RDX")){
		memcpy(pcb->registros_cpu.RDX, valor, 16*sizeof(char));
	}
	sleep(atoi(lectura_de_config.RETARDO_INSTRUCCION));
	return;
}


