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
				//Lee de Memoria y escribe en el registro

				char* registro_mov_in = list_get(instruccion_actual->parametros, 0);
				char* direccion_logica_mov_in = list_get(instruccion_actual->parametros, 1);

				t_datos_mmu datos_mmu_mov_in = mmu(pcb, atoi(direccion_logica_mov_in));

				char** parametros_mov_in = string_array_new();
				string_array_push(&parametros_mov_in, string_itoa(datos_mmu_mov_in.direccion_fisica));
				string_array_push(&parametros_mov_in, string_itoa(tamanio_registro(registro_mov_in)));

				//TODO:
				//-agregar log_acceso_memoria y log(SEG_FAULT)
				//recibir VALOR LEIDO

				enviar_msj_con_parametros(socket_memoria, LEER_VALOR, parametros_mov_in);

				string_array_destroy(parametros_mov_in);

				if(recibir_msj(socket_memoria) == LEIDO_OK) { //No hace falta pero bueno, recibe un mensaje sí o sí
					parametros_mov_in = recibir_parametros_de_mensaje(socket_memoria);
				}

				char* valor_mov_in = parametros_mov_in[0];
				set_registro(pcb, registro_mov_in, valor_mov_in);

				break;

			case MOV_OUT:
				//Lee del registro y escribe en Memoria

				char* direccion_logica_mov_out = list_get(instruccion_actual->parametros, 0);
				char* registro_mov_out = list_get(instruccion_actual->parametros, 1);

				t_datos_mmu datos_mmu_mov_out = mmu(pcb, atoi(direccion_logica_mov_out));

				char** parametros_mov_out = string_array_new();
				string_array_push(&parametros_mov_out, string_itoa(datos_mmu_mov_out.direccion_fisica));
				string_array_push(&parametros_mov_out, string_itoa(tamanio_registro(registro_mov_out))); //No hace falta igual, lo puede calcular memoria con strlen

				char* valor_mov_out = leer_registro(pcb, registro_mov_out);
				string_array_push(&parametros_mov_out, valor_mov_out);
				free(valor_mov_out);

				//TODO:
				//-agregar log_acceso_memoria y log(SEG_FAULT)
				//recibir VALOR ESCRITO

				enviar_msj_con_parametros(socket_memoria, ESCRIBIR_VALOR, parametros_mov_out);

				string_array_destroy(parametros_mov_out);

				if(recibir_msj(socket_memoria) != ESCRITO_OK) { //No hace falta pero bueno, recibe un mensaje sí o sí
					log_error(logger, "Error en el uso de segmentos");
				}

				break;

			case IO:
				enviar_pcb_a_kernel(pcb, IO_EJECUTADO, instruccion_actual->parametros);
				return;

			case F_OPEN:
				enviar_pcb_a_kernel(pcb, F_OPEN_EJECUTADO, instruccion_actual->parametros);
				return;

			case F_CLOSE:
				enviar_pcb_a_kernel(pcb, F_CLOSE_EJECUTADO, instruccion_actual->parametros);
				return;

			case F_SEEK:
				enviar_pcb_a_kernel(pcb, F_SEEK_EJECUTADO, instruccion_actual->parametros);
				return;

			case F_READ: case F_WRITE:
				int direccion_logica = atoi(list_get(instruccion_actual->parametros, 1));

				t_datos_mmu datos_mmu = mmu(pcb, direccion_logica);

				char* cantidad_de_bytes = list_get(instruccion_actual->parametros, 2);
				//if(datos_mmu.desplazamiento_segmento + atoi(cantidad_de_bytes) > tamanio_segmento) { //TODO: calcula mal el tamanio_segmento porque no encuentra el id del segmento pq no se crearon. Ya debería funcionar bien igual.
				if(0) {
					log_info(logger, "PID: %d - Error SEG_FAULT - Segmento: %d - Offset: %d - Tamaño: %d", pcb->pid, datos_mmu.numero_segmento, datos_mmu.desplazamiento_segmento, datos_mmu.tamanio_segmento); //log obligatorio

					enviar_pcb_a_kernel(pcb, EXIT_CON_SEG_FAULT_EJECUTADO, instruccion_actual->parametros);
				}
				else {
					//Me fijo si es F_READ o F_WRITE (hice esto para evitar mucha repetición de lógica)
					t_msj_kernel_cpu mensaje_a_mandar;
					log_acceso_memoria(&mensaje_a_mandar, instruccion_actual->nombre, pcb->pid, datos_mmu.numero_segmento, datos_mmu.direccion_fisica); //log obligatorio

					t_list* parametros_a_enviar = list_create();

					char* str_direccion_fisica = string_itoa(datos_mmu.direccion_fisica);

					list_add(parametros_a_enviar, list_get(instruccion_actual->parametros, 0)); //Nombre del archivo
					list_add(parametros_a_enviar, str_direccion_fisica);
					list_add(parametros_a_enviar, cantidad_de_bytes); //Es un char*

					enviar_pcb_a_kernel(pcb, mensaje_a_mandar, parametros_a_enviar);

					list_destroy(parametros_a_enviar);
					free(str_direccion_fisica);
				}

				return;

			case F_TRUNCATE:
				enviar_pcb_a_kernel(pcb, F_TRUNCATE_EJECUTADO, instruccion_actual->parametros);
				return;

			case WAIT:
				enviar_pcb_a_kernel(pcb, WAIT_EJECUTADO, instruccion_actual->parametros);
				return;

			case SIGNAL:
				enviar_pcb_a_kernel(pcb, SIGNAL_EJECUTADO, instruccion_actual->parametros);
				return;
				break;

			case CREATE_SEGMENT:
				enviar_pcb_a_kernel(pcb, CREATE_SEGMENT_EJECUTADO, instruccion_actual->parametros);
				return;

			case DELETE_SEGMENT:
				enviar_pcb_a_kernel(pcb, DELETE_SEGMENT_EJECUTADO, instruccion_actual->parametros);
				return;

			case YIELD:
				enviar_pcb_a_kernel(pcb, YIELD_EJECUTADO, instruccion_actual->parametros);
				return;

			case EXIT:
				enviar_pcb_a_kernel(pcb, EXIT_EJECUTADO, instruccion_actual->parametros);
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
	char* valor = malloc(tamanio_registro(registro) * sizeof(char) + 1); //+1 para el '\0'

	if(!strcmp(registro, "AX")) {
		memcpy(valor, pcb->registros_cpu.AX, 4 * sizeof(char));
	}
	else if(!strcmp(registro, "BX")) {
		memcpy(valor, pcb->registros_cpu.BX, 4 * sizeof(char));
	}
	else if(!strcmp(registro, "CX")) {
		memcpy(valor, pcb->registros_cpu.CX, 4 * sizeof(char));
	}
	else if(!strcmp(registro, "DX")) {
		memcpy(valor, pcb->registros_cpu.DX, 4 * sizeof(char));
	}
	else if(!strcmp(registro, "EAX")) {
		memcpy(valor, pcb->registros_cpu.EAX, 8 * sizeof(char));
	}
	else if(!strcmp(registro, "EBX")) {
		memcpy(valor, pcb->registros_cpu.EBX, 8 * sizeof(char));
	}
	else if(!strcmp(registro, "ECX")) {
		memcpy(valor, pcb->registros_cpu.ECX, 8 * sizeof(char));
	}
	else if(!strcmp(registro, "EDX")) {
		memcpy(valor, pcb->registros_cpu.EDX, 8 * sizeof(char));
	}
	else if(!strcmp(registro, "RAX")) {
		memcpy(valor, pcb->registros_cpu.RAX, 16 * sizeof(char));
	}
	else if(!strcmp(registro, "RBX")) {
		memcpy(valor, pcb->registros_cpu.RBX, 16 * sizeof(char));
	}
	else if(!strcmp(registro, "RCX")) {
		memcpy(valor, pcb->registros_cpu.RCX, 16 * sizeof(char));
	}
	else if(!strcmp(registro, "RDX")) {
		memcpy(valor, pcb->registros_cpu.RDX, 16 * sizeof(char));
	}

	char barra_cero = '\0';
	memcpy(valor + tamanio_registro(registro) + 1, &barra_cero, sizeof(char)); //Para que pueda funcionar bien con las funciones de string_array

	usleep(lectura_de_config.RETARDO_INSTRUCCION * 1000);
	return valor;
}

int tamanio_registro(char* nombre_registro) {
	if(strlen(nombre_registro) == 2) {
		return 4;
	}
	if(nombre_registro[0] == 'E') {
		return 8;
	}
	return 16;
}

void enviar_pcb_a_kernel(t_pcb* pcb, t_msj_kernel_cpu mensaje, t_list* list_parametros) {
	char** parametros = string_array_new();

	for(int i = 0; i < list_size(list_parametros); i++) {
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

int buscar_campo_de_segmento(t_list* segmentos, char* campo, int id) {
	t_segmento* segmento;

	for(int i = 0; i < list_size(segmentos); i++) {
		segmento = list_get(segmentos, i);
		if(segmento->id == id) {
			if(!strcmp(campo, "base")) {
				return segmento->tamanio;
			}
			if(!strcmp(campo, "tamanio")) {
				return segmento->tamanio;
			}
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

t_datos_mmu mmu(t_pcb* pcb, int direccion_logica) {
	t_datos_mmu datos;

	datos.numero_segmento = direccion_logica / lectura_de_config.TAM_MAX_SEGMENTO; //al asignarle a un int se obtiene el floor
	datos.desplazamiento_segmento = direccion_logica % lectura_de_config.TAM_MAX_SEGMENTO;

	datos.tamanio_segmento = buscar_campo_de_segmento(pcb->tabla_segmentos, "tamanio", datos.numero_segmento);

	datos.direccion_fisica = datos.desplazamiento_segmento + buscar_campo_de_segmento(pcb->tabla_segmentos, "base", datos.numero_segmento);

	return datos;
}
