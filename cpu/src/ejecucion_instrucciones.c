#include "../include/ejecucion_instrucciones.h"

t_log* logger;

void ejecutar_instrucciones(t_pcb* pcb) {
	int cantidad_instrucciones = list_size(pcb->instrucciones);
	t_instruccion* instruccion_actual;
	char** parametros = string_array_new();

	while(pcb->pc < cantidad_instrucciones) {
		//Fetch
		instruccion_actual = list_get(pcb->instrucciones, pcb->pc);
		pcb->pc++;
		log_info(logger, "PID: %d - Ejecutando: %s - <PARAMETROS>", pcb->pid, instruccion_actual->nombre); //log obligatorio
		//TODO: emitir los parametros
		//Decode y Execute
		//TODO: descomentar todovich (me lo marca como task si lo escribo sin vich jasjasj)
		switch(instruccion_a_enum(instruccion_actual)) {
			case SET:
				ejecutar_set(pcb, instruccion_actual);
				break;

			case MOV_IN:
				//parametros = malloc(2 * sizeof(char*));
				//parametros[0] = strdup(list_get(instruccion_actual->parametros, 0));
				//parametros[1] = strdup(list_get(instruccion_actual->parametros, 1));
				//liberar_parametros(parametros);
				string_array_push(&parametros, list_get(instruccion_actual->parametros, 0));
				string_array_push(&parametros, list_get(instruccion_actual->parametros, 1));

				//TODO: IMPLEMENTAR

				break;

			case MOV_OUT:
				//parametros = malloc(2 * sizeof(char*));
				//parametros[0] = strdup(list_get(instruccion_actual->parametros, 0));
				//parametros[1] = strdup(list_get(instruccion_actual->parametros, 1));
				string_array_push(&parametros, list_get(instruccion_actual->parametros, 0));
				string_array_push(&parametros, list_get(instruccion_actual->parametros, 1));

				//TODO: IMPLEMENTAR

				//liberar_parametros(parametros);
				string_array_destroy(parametros);
				break;

			case IO:
				//parametros = malloc(sizeof(char*));
				//parametros[0] = strdup(list_get(instruccion_actual->parametros, 0)); //va con strdup?
				//string_array_push(&parametros, list_get(instruccion_actual->parametros, 0));

				//enviar_pcb(socket_kernel, pcb, IO_EJECUTADO, parametros);
				//liberar_parametros(parametros);

				enviar_pcb_a_kernel(pcb, IO_EJECUTADO, parametros, instruccion_actual->parametros, 1);
				return;
				break;

			case F_OPEN:
				//parametros = malloc(sizeof(char*));
				//parametros[0] = strdup(list_get(instruccion_actual->parametros, 0));
				//enviar_pcb(socket_kernel, pcb, F_OPEN_EJECUTADO, parametros);
				//liberar_parametros(parametros);

				enviar_pcb_a_kernel(pcb, F_OPEN_EJECUTADO, parametros, instruccion_actual->parametros, 1);
				return;
				break;

			case F_CLOSE:
				//parametros = malloc(sizeof(char*));
				//parametros[0] = strdup(list_get(instruccion_actual->parametros, 0));
				//enviar_pcb(socket_kernel, pcb, F_CLOSE_EJECUTADO, parametros);
				//liberar_parametros(parametros);

				enviar_pcb_a_kernel(pcb, F_CLOSE_EJECUTADO, parametros, instruccion_actual->parametros, 1);
				return;
				break;

			case F_SEEK:
				//parametros = malloc(2 * sizeof(char*));
				//parametros[0] = strdup(list_get(instruccion_actual->parametros, 0));
				//parametros[1] = strdup(list_get(instruccion_actual->parametros, 1));
				//enviar_pcb(socket_kernel, pcb, F_SEEK_EJECUTADO, parametros);
				//liberar_parametros(parametros);

				enviar_pcb_a_kernel(pcb, F_SEEK_EJECUTADO, parametros, instruccion_actual->parametros, 2);
				//log_warning(logger, "F_SEEK_EJECUTADO = %d", F_SEEK_EJECUTADO);
				return;
				break;

			case F_READ:
				//parametros = malloc(3 * sizeof(char*));
				//parametros[0] = strdup(list_get(instruccion_actual->parametros, 0));
				//parametros[1] = strdup(list_get(instruccion_actual->parametros, 1));
				//parametros[2] = strdup(list_get(instruccion_actual->parametros, 2));
				//enviar_pcb(socket_kernel, pcb, F_READ_EJECUTADO, parametros);
				//liberar_parametros(parametros);

				enviar_pcb_a_kernel(pcb, F_READ_EJECUTADO, parametros, instruccion_actual->parametros, 3);
				return;
				break;

			case F_WRITE:
				//parametros = malloc(3 * sizeof(char*));
				//parametros[0] = strdup(list_get(instruccion_actual->parametros, 0));
				//parametros[1] = strdup(list_get(instruccion_actual->parametros, 1));
				//parametros[2] = strdup(list_get(instruccion_actual->parametros, 2));
				//enviar_pcb(socket_kernel, pcb, F_WRITE_EJECUTADO, parametros);
				//liberar_parametros(parametros);

				enviar_pcb_a_kernel(pcb, F_WRITE_EJECUTADO, parametros, instruccion_actual->parametros, 3);
				return;
				break;

			case F_TRUNCATE:
				//parametros = malloc(2 * sizeof(char*));
				//parametros[0] = strdup(list_get(instruccion_actual->parametros, 0));
				//parametros[1] = strdup(list_get(instruccion_actual->parametros, 1));
				//enviar_pcb(socket_kernel, pcb, F_TRUNCATE_EJECUTADO, parametros);
				//liberar_parametros(parametros);

				enviar_pcb_a_kernel(pcb, F_TRUNCATE_EJECUTADO, parametros, instruccion_actual->parametros, 2);
				return;
				break;

			case WAIT:
				//parametros = malloc(sizeof(char*));
				//parametros[0] = strdup(list_get(instruccion_actual->parametros, 0)); //recurso_a_usar //va con strdup?
				//string_array_push(&parametros, list_get(instruccion_actual->parametros, 0));

				//enviar_pcb(socket_kernel, pcb, WAIT_EJECUTADO, parametros);
				//liberar_parametros(parametros);

				enviar_pcb_a_kernel(pcb, WAIT_EJECUTADO, parametros, instruccion_actual->parametros, 1);
				return;
				break;

			case SIGNAL:
				//char* recurso_a_desbloquear = list_get(instruccion_actual->parametros, 0); //va con strdup?
				//string_array_push(&parametros, recurso_a_desbloquear);
				//parametros = malloc(sizeof(char*));
				//parametros[0] = strdup(list_get(instruccion_actual->parametros, 0)); //recurso_a_desbloquear //va con strdup?
				//enviar_pcb(socket_kernel, pcb, SIGNAL_EJECUTADO, parametros);
				//liberar_parametros(parametros);

				enviar_pcb_a_kernel(pcb, SIGNAL_EJECUTADO, parametros, instruccion_actual->parametros, 1);
				return;
				break;

			case CREATE_SEGMENT:
				//parametros = malloc(2 * sizeof(char*));
				//parametros[0] = strdup(list_get(instruccion_actual->parametros, 0));
				//parametros[1] = strdup(list_get(instruccion_actual->parametros, 1));
				//enviar_pcb(socket_kernel, pcb, CREATE_SEGMENT_EJECUTADO, parametros);
				//liberar_parametros(parametros);

				enviar_pcb_a_kernel(pcb, CREATE_SEGMENT_EJECUTADO, parametros, instruccion_actual->parametros, 2);
				return;
				break;

			case DELETE_SEGMENT:
				//parametros = malloc(sizeof(char*));
				//parametros[0] = strdup(list_get(instruccion_actual->parametros, 0));
				//enviar_pcb(socket_kernel, pcb, DELETE_SEGMENT_EJECUTADO, parametros);
				//liberar_parametros(parametros);

				enviar_pcb_a_kernel(pcb, DELETE_SEGMENT_EJECUTADO, parametros, instruccion_actual->parametros, 1);
				return;
				break;

			case YIELD:
				enviar_pcb_a_kernel(pcb, YIELD_EJECUTADO, parametros, instruccion_actual->parametros, 0);
				return;
				break;

			case EXIT:
				enviar_pcb_a_kernel(pcb, EXIT_EJECUTADO, parametros, instruccion_actual->parametros, 0);
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

void enviar_pcb_a_kernel(t_pcb* pcb, t_msj_kernel_cpu mensaje, char** parametros, t_list* list_parametros, int cantidad_de_parametros) {
	for(int i = 0; i < cantidad_de_parametros; i++) {
		string_array_push(&parametros, list_get(list_parametros, i));
	}

	enviar_pcb(socket_kernel, pcb, mensaje, parametros);

	//string_array_destroy(parametros); //TODO: valgrind dice que estoy haciendo 2 veces free
}


