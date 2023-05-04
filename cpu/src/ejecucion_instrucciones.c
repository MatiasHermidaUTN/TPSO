#include "../include/ejecucion_instrucciones.h"

t_log* logger;

void ejecutar_instrucciones(t_pcb* pcb) {
	int cantidad_instrucciones = list_size(pcb->instrucciones);
	t_instruccion* instruccion_actual;
	char** parametros;// = string_array_new();

	while(pcb->pc < cantidad_instrucciones) {
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
				parametros = malloc(sizeof(char*) * 2);
				parametros[0] = list_get(instruccion_actual->parametros,0);
				parametros[1] = list_get(instruccion_actual->parametros,1);

				//TODO: IMPLEMENTAR

				liberar_parametros(parametros);
				break;

			case MOV_OUT:
				parametros = malloc(sizeof(char*) * 2);
				parametros[0] = list_get(instruccion_actual->parametros,0);
				parametros[1] = list_get(instruccion_actual->parametros,1);

				//TODO: IMPLEMENTAR

				liberar_parametros(parametros);
				break;

			case IO:
				//char* tiempo_a_bloquear = list_get(instruccion_actual->parametros, 0); //va con strdup?
				//string_array_push(&parametros, tiempo_a_bloquear);
				parametros = malloc(sizeof(char*));
				parametros[0] = list_get(instruccion_actual->parametros, 0);
				enviar_pcb(socket_kernel, pcb, IO_EJECUTADO, parametros);
				free(parametros);
				return;
				break;

			case WAIT:
				//char* recurso_a_usar = list_get(instruccion_actual->parametros, 0); //va con strdup?
				//string_array_push(&parametros, recurso_a_usar);
				parametros = malloc(sizeof(char*));
				parametros[0] = list_get(instruccion_actual->parametros, 0); //recurso_a_usar //va con strdup?
				enviar_pcb(socket_kernel, pcb, WAIT_EJECUTADO, parametros);
				free(parametros);
				return;
				break;

			case SIGNAL:
				//char* recurso_a_desbloquear = list_get(instruccion_actual->parametros, 0); //va con strdup?
				//string_array_push(&parametros, recurso_a_desbloquear);
				parametros = malloc(sizeof(char*));
				parametros[0] = list_get(instruccion_actual->parametros, 0); //recurso_a_desbloquear //va con strdup?
				enviar_pcb(socket_kernel, pcb, SIGNAL_EJECUTADO, parametros);
				free(parametros);
				return;
				break;

			case CREATE_SEGMENT:
				parametros = malloc(sizeof(char*) * 2);
				parametros[0] = list_get(instruccion_actual->parametros,0);
				parametros[1] = list_get(instruccion_actual->parametros,1);
				enviar_pcb(socket_kernel, pcb, CREATE_SEGMENT_EJECUTADO, parametros);
				free(parametros);
				return;
				break;

			case DELETE_SEGMENT:
				parametros = malloc(sizeof(char*));
				parametros[0] = list_get(instruccion_actual->parametros,0);
				enviar_pcb(socket_kernel, pcb, DELETE_SEGMENT_EJECUTADO, parametros);
				free(parametros);
				return;
				break;

			case YIELD:
				enviar_pcb(socket_kernel, pcb, YIELD_EJECUTADO, NULL); //NULL porque no se le pasa ningun parametro
				return;
				break;

			case F_OPEN:
				parametros = malloc(sizeof(char*));
				parametros[0] = list_get(instruccion_actual->parametros,0);
				enviar_pcb(socket_kernel, pcb, F_OPEN_EJECUTADO, parametros);
				free(parametros);
				return;
				break;

			case F_CLOSE:
				parametros = malloc(sizeof(char*));
				parametros[0] = list_get(instruccion_actual->parametros,0);
				enviar_pcb(socket_kernel, pcb, F_CLOSE_EJECUTADO, parametros);
				free(parametros);
				return;
				break;

			case F_SEEK:
				parametros = malloc(sizeof(char*)*2);
				parametros[0] = list_get(instruccion_actual->parametros,0);
				parametros[1] = list_get(instruccion_actual->parametros,1);
				enviar_pcb(socket_kernel, pcb, F_SEEK_EJECUTADO, parametros);
				free(parametros);
				return;
				break;

			case F_READ:
				parametros = malloc(sizeof(char*)*3);
				parametros[0] = list_get(instruccion_actual->parametros,0);
				parametros[1] = list_get(instruccion_actual->parametros,1);
				parametros[2] = list_get(instruccion_actual->parametros,2);
				enviar_pcb(socket_kernel, pcb, F_READ_EJECUTADO, parametros);
				free(parametros);
				return;
				break;

			case F_WRITE:
				parametros = malloc(sizeof(char*)*3);
				parametros[0] = list_get(instruccion_actual->parametros,0);
				parametros[1] = list_get(instruccion_actual->parametros,1);
				parametros[2] = list_get(instruccion_actual->parametros,2);
				enviar_pcb(socket_kernel, pcb, F_WRITE_EJECUTADO, parametros);
				free(parametros);
				return;
				break;

			case F_TRUNCATE:
				parametros = malloc(sizeof(char*)*2);
				parametros[0] = list_get(instruccion_actual->parametros,0);
				parametros[1] = list_get(instruccion_actual->parametros,1);
				enviar_pcb(socket_kernel, pcb, F_TRUNCATE_EJECUTADO, parametros);
				free(parametros);
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

t_enum_instruccion instruccion_a_enum(t_instruccion* instruccion) {
	char* nombre = instruccion->nombre;
	if(!strcmp(nombre, "SET")) 			  return SET;
	if(!strcmp(nombre, "MOV_IN")) 	      return MOV_IN;
	if(!strcmp(nombre, "MOV_OUT")) 		  return MOV_OUT;
	if(!strcmp(nombre, "I/O")) 		  	  return IO;
	if(!strcmp(nombre, "F_OPEN"))		  return F_OPEN;
	if(!strcmp(nombre, "F_CLOSE")) 		  return F_CLOSE;
	if(!strcmp(nombre, "F_SEEK")) 		  return F_SEEK;
	if(!strcmp(nombre, "F_READ")) 		  return F_READ;
	if(!strcmp(nombre, "F_WRITE")) 		  return F_WRITE;
	if(!strcmp(nombre, "F_TRUNCATE")) 	  return F_TRUNCATE;
	if(!strcmp(nombre, "WAIT")) 		  return WAIT;
	if(!strcmp(nombre, "SIGNAL")) 		  return SIGNAL;
	if(!strcmp(nombre, "CREATE_SEGMENT")) return CREATE_SEGMENT;
	if(!strcmp(nombre, "DELETE_SEGMENT")) return DELETE_SEGMENT;
	if(!strcmp(nombre, "YIELD")) 		  return YIELD;
	if(!strcmp(nombre, "EXIT")) 		  return EXIT;
	return INSTRUCCION_ERRONEA;
}

void ejecutar_set(t_pcb* pcb, t_instruccion* instruccion_actual) {
	char* registro = list_get(instruccion_actual->parametros, 0);
	char* valor    = list_get(instruccion_actual->parametros, 1);

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


