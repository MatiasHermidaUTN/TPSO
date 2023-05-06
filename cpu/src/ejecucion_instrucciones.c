#include "../include/ejecucion_instrucciones.h"

t_log* logger;

void ejecutar_instrucciones(t_pcb* pcb) {
	int cantidad_instrucciones = list_size(pcb->instrucciones);
	t_instruccion* instruccion_actual;

	while(pcb->pc < cantidad_instrucciones) {
		//Fetch
		instruccion_actual = list_get(pcb->instrucciones, pcb->pc);
		pcb->pc++;
		log_info(logger, "PID: %d - Ejecutando: %s - <PARAMETROS>", pcb->pid, instruccion_actual->nombre); //log obligatorio
		//TODO: emitir los parametros
		//Decode y Execute
		switch(instruccion_a_enum(instruccion_actual)) {
			case SET:
				log_warning(logger, "AX: %c", pcb->registros_cpu.AX[0]);
				log_warning(logger, "AX: %c", pcb->registros_cpu.AX[1]);
				log_warning(logger, "AX: %c", pcb->registros_cpu.AX[2]);
				log_warning(logger, "AX: %c", pcb->registros_cpu.AX[3]);
				//log_warning(logger, "AX: %s", pcb->registros_cpu.AX); //Valgrind tira warning de conditional jump
				ejecutar_set(pcb, instruccion_actual);
				log_warning(logger, "AX: %c", pcb->registros_cpu.AX[0]);
				log_warning(logger, "AX: %c", pcb->registros_cpu.AX[1]);
				log_warning(logger, "AX: %c", pcb->registros_cpu.AX[2]);
				log_warning(logger, "AX: %c", pcb->registros_cpu.AX[3]);
				//log_warning(logger, "AX: %s", pcb->registros_cpu.AX);

				break;

			case MOV_IN:
				char* registro_mov_in = list_get(instruccion_actual->parametros, 0);
				char* direccion_logica_mov_in = list_get(instruccion_actual->parametros, 1);

				//TODO: IMPLEMENTAR
				break;

			case MOV_OUT:
				char* registro_mov_out  = list_get(instruccion_actual->parametros, 0);
				char* direccion_logica_mov_out = list_get(instruccion_actual->parametros, 1);

				//TODO: IMPLEMENTAR
				break;

			case IO:
				enviar_pcb_a_kernel(pcb, IO_EJECUTADO, instruccion_actual->parametros, 1);
				return;
				break;

			case F_OPEN:
				enviar_pcb_a_kernel(pcb, F_OPEN_EJECUTADO, instruccion_actual->parametros, 1);
				return;
				break;

			case F_CLOSE:
				enviar_pcb_a_kernel(pcb, F_CLOSE_EJECUTADO, instruccion_actual->parametros, 1);
				return;
				break;

			case F_SEEK:
				enviar_pcb_a_kernel(pcb, F_SEEK_EJECUTADO, instruccion_actual->parametros, 2);
				return;
				break;

			case F_READ:
				enviar_pcb_a_kernel(pcb, F_READ_EJECUTADO, instruccion_actual->parametros, 3);
				return;
				break;

			case F_WRITE:
				enviar_pcb_a_kernel(pcb, F_WRITE_EJECUTADO, instruccion_actual->parametros, 3);
				return;
				break;

			case F_TRUNCATE:
				enviar_pcb_a_kernel(pcb, F_TRUNCATE_EJECUTADO, instruccion_actual->parametros, 2);
				return;
				break;

			case WAIT:
				enviar_pcb_a_kernel(pcb, WAIT_EJECUTADO, instruccion_actual->parametros, 1);
				return;
				break;

			case SIGNAL:
				enviar_pcb_a_kernel(pcb, SIGNAL_EJECUTADO, instruccion_actual->parametros, 1);
				return;
				break;

			case CREATE_SEGMENT:
				enviar_pcb_a_kernel(pcb, CREATE_SEGMENT_EJECUTADO, instruccion_actual->parametros, 2);
				return;
				break;

			case DELETE_SEGMENT:
				enviar_pcb_a_kernel(pcb, DELETE_SEGMENT_EJECUTADO, instruccion_actual->parametros, 1);
				return;
				break;

			case YIELD:
				enviar_pcb_a_kernel(pcb, YIELD_EJECUTADO, instruccion_actual->parametros, 0);
				return;
				break;

			case EXIT:
				enviar_pcb_a_kernel(pcb, EXIT_EJECUTADO, instruccion_actual->parametros, 0);
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
	usleep(atoi(lectura_de_config.RETARDO_INSTRUCCION) * 1000);
	return;
}

void enviar_pcb_a_kernel(t_pcb* pcb, t_msj_kernel_cpu mensaje, t_list* list_parametros, int cantidad_de_parametros) {
	char** parametros = string_array_new();

	for(int i = 0; i < cantidad_de_parametros; i++) {
		string_array_push(&parametros, list_get(list_parametros, i));
	}

	enviar_pcb(socket_kernel, pcb, mensaje, parametros);

	free(parametros); //TODO: valgrind dice que estoy haciendo 2 veces free
	//hay que hacer un free y no un string_array_destroy porque sino estraría liberando los de el pcb
}


