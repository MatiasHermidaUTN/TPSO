#include "../include/ejecucion_instrucciones.h"

t_log* logger;

void ejecutar_instrucciones(t_pcb* pcb) {
	int cantidad_instrucciones = list_size(pcb->instrucciones);
	t_instruccion* instruccion_actual;

	while(pcb->pc < cantidad_instrucciones) {
		//Fetch
		instruccion_actual = list_get(pcb->instrucciones, pcb->pc);
		pcb->pc++;

		char* parametros_a_emitir = obtener_parametros_a_emitir(instruccion_actual->parametros);
		log_info(logger, "PID: %d - Ejecutando: %s - %s", pcb->pid, instruccion_actual->nombre, parametros_a_emitir); //log obligatorio
		free(parametros_a_emitir);

		//Decode y Execute
		switch(instruccion_a_enum(instruccion_actual)) {
			case SET:
				//log_warning(logger, "AX: %c%c%c%c", pcb->registros_cpu.AX[0], pcb->registros_cpu.AX[1], pcb->registros_cpu.AX[2], pcb->registros_cpu.AX[3]);
				set_registro(pcb, list_get(instruccion_actual->parametros, 0), list_get(instruccion_actual->parametros, 1));
				//log_warning(logger, "AX: %c%c%c%c", pcb->registros_cpu.AX[0], pcb->registros_cpu.AX[1], pcb->registros_cpu.AX[2], pcb->registros_cpu.AX[3]);

				break;

			case MOV_IN:
				char* registro_mov_in = list_get(instruccion_actual->parametros, 0);
				char* direccion_logica_mov_in = list_get(instruccion_actual->parametros, 1);

				char** parametros_mov_in = string_array_new();
				string_array_push(&parametros_mov_in, direccion_logica_mov_in); //TODO: fijarse si se envía la dirección lógica o física

				enviar_msj_con_parametros(socket_memoria, LEER_VALOR, parametros_mov_in);

				string_array_destroy(parametros_mov_in);

				parametros_mov_in = recibir_parametros_de_mensaje(socket_memoria); //TODO: fijarse si memoria debería tener dos hilos: uno para escuchar a FS y otro para CPU

				char* valor_mov_in = parametros_mov_in[0];
				set_registro(pcb, registro_mov_in, valor_mov_in);

				break;

			case MOV_OUT:
				char* direccion_logica_mov_out = list_get(instruccion_actual->parametros, 0);
				char* registro_mov_out = list_get(instruccion_actual->parametros, 1);

				char** parametros_mov_out = string_array_new();
				string_array_push(&parametros_mov_out, direccion_logica_mov_out); //TODO: fijarse si se envía la dirección lógica o física

				char* valor_mov_out = leer_registro(pcb, registro_mov_out);
				string_array_push(&parametros_mov_out, valor_mov_out);
				free(valor_mov_out);

				enviar_msj_con_parametros(socket_memoria, ESCRIBIR_VALOR, parametros_mov_out);

				string_array_destroy(parametros_mov_out);

				break;

			case IO:
				enviar_pcb_a_kernel(pcb, IO_EJECUTADO, instruccion_actual->parametros, 1);
				return;

			case F_OPEN:
				enviar_pcb_a_kernel(pcb, F_OPEN_EJECUTADO, instruccion_actual->parametros, 1);
				return;

			case F_CLOSE:
				enviar_pcb_a_kernel(pcb, F_CLOSE_EJECUTADO, instruccion_actual->parametros, 1);
				return;

			case F_SEEK:
				enviar_pcb_a_kernel(pcb, F_SEEK_EJECUTADO, instruccion_actual->parametros, 2);
				return;

			case F_READ: case F_WRITE:
				//MMU
				int direccion_logica = atoi(list_get(instruccion_actual->parametros, 1)); //TODO: fijarse si es hexadecimal y no int

				int numero_segmento = direccion_logica / lectura_de_config.TAM_MAX_SEGMENTO; //al asignarle a un int se obtiene el floor
				int desplazamiento_segmento = direccion_logica % lectura_de_config.TAM_MAX_SEGMENTO;
				int tamanio_segmento = buscar_tamanio_segmento(pcb->tabla_segmentos, numero_segmento);

				int direccion_fisica = tamanio_segmento * numero_segmento + desplazamiento_segmento;


				//Me fijo si es F_READ o F_WRITE (hice esto para evitar mucha repetición de lógica)
				t_msj_kernel_cpu mensaje_a_mandar;
				log_acceso_memoria(&mensaje_a_mandar, instruccion_actual->nombre, pcb->pid, numero_segmento, direccion_fisica); //log obligatorio

				int offset_total = desplazamiento_segmento + atoi(list_get(instruccion_actual->parametros, 2));
				//if(offset_total > tamanio_segmento) { //TODO: calcula mal el tamanio_segmento porque no encuentra el id del segmento. Resolver duda sobre quién crea la tabla de segmentos (si Memoria, quien debería manejar todo, o Kernel con t_list y fue)
				if(0) {
					log_info(logger, "PID: %d - Error SEG_FAULT - Segmento: %d - Offset: %d - Tamaño: %d", pcb->pid, numero_segmento, offset_total - tamanio_segmento, tamanio_segmento); //log obligatorio
					//TODO: fijarse si es correcto el OFFSET

					enviar_pcb_a_kernel(pcb, EXIT_CON_SEG_FAULT_EJECUTADO, instruccion_actual->parametros, 3);
				}
				else {
					t_list* parametros_a_enviar = list_create();

					char* str_direccion_fisica = string_itoa(direccion_fisica);

					list_add(parametros_a_enviar, list_get(instruccion_actual->parametros, 0));
					list_add(parametros_a_enviar, str_direccion_fisica);
					list_add(parametros_a_enviar, list_get(instruccion_actual->parametros, 2));

					enviar_pcb_a_kernel(pcb, mensaje_a_mandar, parametros_a_enviar, 3);

					list_destroy(parametros_a_enviar);
					free(str_direccion_fisica);
				}

				return;

			case F_TRUNCATE:
				enviar_pcb_a_kernel(pcb, F_TRUNCATE_EJECUTADO, instruccion_actual->parametros, 2);
				return;

			case WAIT:
				enviar_pcb_a_kernel(pcb, WAIT_EJECUTADO, instruccion_actual->parametros, 1);
				return;

			case SIGNAL:
				enviar_pcb_a_kernel(pcb, SIGNAL_EJECUTADO, instruccion_actual->parametros, 1);
				return;
				break;

			case CREATE_SEGMENT:
				enviar_pcb_a_kernel(pcb, CREATE_SEGMENT_EJECUTADO, instruccion_actual->parametros, 2);
				return;

			case DELETE_SEGMENT:
				enviar_pcb_a_kernel(pcb, DELETE_SEGMENT_EJECUTADO, instruccion_actual->parametros, 1);
				return;

			case YIELD:
				enviar_pcb_a_kernel(pcb, YIELD_EJECUTADO, instruccion_actual->parametros, 0);
				return;

			case EXIT:
				enviar_pcb_a_kernel(pcb, EXIT_EJECUTADO, instruccion_actual->parametros, 0);
				return;

			default:
				log_error(logger, "Error en el decode de la instruccion");
				//exit(EXIT_FAILURE);
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

void set_registro(t_pcb* pcb, char* registro, char* valor) {
	if(!strcmp(registro, "AX")) {
		memcpy(pcb->registros_cpu.AX, valor, 4 * sizeof(char));
	}
	else if(!strcmp(registro, "BX")) {
		memcpy(pcb->registros_cpu.BX, valor, 4 * sizeof(char));
	}
	else if(!strcmp(registro, "CX")) {
		memcpy(pcb->registros_cpu.CX, valor, 4 * sizeof(char));
	}
	else if(!strcmp(registro, "DX")) {
		memcpy(pcb->registros_cpu.DX, valor, 4 * sizeof(char));
	}
	else if(!strcmp(registro, "EAX")) {
		memcpy(pcb->registros_cpu.EAX, valor, 8 * sizeof(char));
	}
	else if(!strcmp(registro, "EBX")) {
		memcpy(pcb->registros_cpu.EBX, valor, 8 * sizeof(char));
	}
	else if(!strcmp(registro, "ECX")) {
		memcpy(pcb->registros_cpu.ECX, valor, 8 * sizeof(char));
	}
	else if(!strcmp(registro, "EDX")) {
		memcpy(pcb->registros_cpu.EDX, valor, 8 * sizeof(char));
	}
	else if(!strcmp(registro, "RAX")) {
		memcpy(pcb->registros_cpu.RAX, valor, 16 * sizeof(char));
	}
	else if(!strcmp(registro, "RBX")) {
		memcpy(pcb->registros_cpu.RBX, valor, 16 * sizeof(char));
	}
	else if(!strcmp(registro, "RCX")) {
		memcpy(pcb->registros_cpu.RCX, valor, 16 * sizeof(char));
	}
	else if(!strcmp(registro, "RDX")) {
		memcpy(pcb->registros_cpu.RDX, valor, 16 * sizeof(char));
	}

	usleep(lectura_de_config.RETARDO_INSTRUCCION * 1000);
	return;
}

char* leer_registro(t_pcb* pcb, char* registro) {
	char* valor;

	if(!strcmp(registro, "AX")) {
		valor = malloc(4 * sizeof(char));
		memcpy(valor, pcb->registros_cpu.AX, 4 * sizeof(char));
	}
	else if(!strcmp(registro, "BX")) {
		valor = malloc(4 * sizeof(char));
		memcpy(valor, pcb->registros_cpu.BX, 4 * sizeof(char));
	}
	else if(!strcmp(registro, "CX")) {
		valor = malloc(4 * sizeof(char));
		memcpy(valor, pcb->registros_cpu.CX, 4 * sizeof(char));
	}
	else if(!strcmp(registro, "DX")) {
		valor = malloc(4 * sizeof(char));
		memcpy(valor, pcb->registros_cpu.DX, 4 * sizeof(char));
	}
	else if(!strcmp(registro, "EAX")) {
		valor = malloc(8 * sizeof(char));
		memcpy(valor, pcb->registros_cpu.EAX, 8 * sizeof(char));
	}
	else if(!strcmp(registro, "EBX")) {
		valor = malloc(8 * sizeof(char));
		memcpy(valor, pcb->registros_cpu.EBX, 8 * sizeof(char));
	}
	else if(!strcmp(registro, "ECX")) {
		valor = malloc(8 * sizeof(char));
		memcpy(valor, pcb->registros_cpu.ECX, 8 * sizeof(char));
	}
	else if(!strcmp(registro, "EDX")) {
		valor = malloc(8 * sizeof(char));
		memcpy(valor, pcb->registros_cpu.EDX, 8 * sizeof(char));
	}
	else if(!strcmp(registro, "RAX")) {
		valor = malloc(16 * sizeof(char));
		memcpy(valor, pcb->registros_cpu.RAX, 16 * sizeof(char));
	}
	else if(!strcmp(registro, "RBX")) {
		valor = malloc(16 * sizeof(char));
		memcpy(valor, pcb->registros_cpu.RBX, 16 * sizeof(char));
	}
	else if(!strcmp(registro, "RCX")) {
		valor = malloc(16 * sizeof(char));
		memcpy(valor, pcb->registros_cpu.RCX, 16 * sizeof(char));
	}
	else if(!strcmp(registro, "RDX")) {
		valor = malloc(16 * sizeof(char));
		memcpy(valor, pcb->registros_cpu.RDX, 16 * sizeof(char));
	}

	usleep(lectura_de_config.RETARDO_INSTRUCCION * 1000);
	return valor;
}

void enviar_pcb_a_kernel(t_pcb* pcb, t_msj_kernel_cpu mensaje, t_list* list_parametros, int cantidad_de_parametros) {
	char** parametros = string_array_new();

	for(int i = 0; i < cantidad_de_parametros; i++) {
		string_array_push(&parametros, list_get(list_parametros, i));
	}

	enviar_pcb(socket_kernel, pcb, mensaje, parametros);

	free(parametros);
	//hay que hacer un free y no un string_array_destroy porque sino estraría liberando los de el pcb
}

char* obtener_parametros_a_emitir(t_list* parametros_actuales) {
	char* parametros_a_emitir = string_new();
	for(int i = 0; i < list_size(parametros_actuales); i++) {
		string_append(&parametros_a_emitir, list_get(parametros_actuales, i));
		if(i != list_size(parametros_actuales) - 1) {
			string_append(&parametros_a_emitir, " ");
		}
	}

	return parametros_a_emitir;
}

int buscar_tamanio_segmento(t_list* segmentos, int id) {
	t_segmento* segmento;

	for(int i = 0; i < list_size(segmentos); i++) {
		segmento = list_get(segmentos, i);
		if(segmento->id == id) {
			return segmento->tamanio;
		}
	}

	//TODO: Entra por acá siempre pues no coincide el id (ya que todavía no implementamos la creación de segmentos desde Memoria)
	return -1;
}

void log_acceso_memoria(t_msj_kernel_cpu* mensaje_a_mandar, char* nombre_instruccion, int pid, int numero_segmento, int direccion_fisica) {
	char* accion;
	if(!strcmp(nombre_instruccion, "F_READ")) {
		*mensaje_a_mandar = F_READ_EJECUTADO;
		accion = strdup("LEER");
	}
	else { //F_WRITE
		*mensaje_a_mandar = F_WRITE_EJECUTADO;
		accion = strdup("ESCRIBIR");
	}

	log_info(logger, "PID: %d - Acción: %s - Segmento: %d - Dirección Física: %d - Valor: <VALOR LEIDO / ESCRITO>", pid, accion, numero_segmento, direccion_fisica); //log obligatorio
	//TODO: falta <VALOR LEIDO / ESCRITO>
	free(accion);
}

