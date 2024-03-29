#include "../include/ejecucion_instrucciones.h"

void ejecutar_instrucciones(t_pcb* pcb) {
	int cantidad_instrucciones = list_size(pcb->instrucciones);
	t_instruccion* instruccion_actual_t;

	while(pcb->pc < cantidad_instrucciones) {
		///////////
		// Fetch //
		///////////
		instruccion_actual_t = list_get(pcb->instrucciones, pcb->pc);
		pcb->pc++;

		t_list* parametros_actuales = instruccion_actual_t->parametros;

		char* parametros_a_emitir = obtener_parametros_a_emitir(parametros_actuales);
		log_info(logger, "PID: %d - Ejecutando: %s - %s", pcb->pid, instruccion_actual_t->nombre, parametros_a_emitir); //log obligatorio
		free(parametros_a_emitir);

		////////////
		// Decode //
		////////////
		t_msj_kernel_cpu instruccion_actual = instruccion_a_enum(instruccion_actual_t);

		/////////////
		// Execute //
		/////////////
		switch(instruccion_actual) {
			case SET:
				set_registro(pcb, list_get(parametros_actuales, 0), list_get(parametros_actuales, 1));
				break;

			case MOV_IN: case MOV_OUT: case F_READ: case F_WRITE:
				/*
				MOV_IN lee de Memoria y escribe en registro
				MOV_OUT lee el registro y escribe en Memoria

				F_READ lee del archivo y escribe en Memoria
				F_WRITE lee de Memoria y escribe en archivo
				*/

				// OBTENCION DE DATOS
				int indice = numero_de_parametro_de_direccion_logica(instruccion_actual);
				int direccion_logica = atoi(list_get(parametros_actuales, indice));

				t_datos_mmu datos_mmu = mmu(pcb, direccion_logica);

				int cantidad_de_bytes = obtener_cantidad_de_bytes(instruccion_actual, parametros_actuales);

				// EVALUACION SEG_FAULT
				if(datos_mmu.desplazamiento_segmento + cantidad_de_bytes > datos_mmu.tamanio_segmento) {
					log_info(logger, "PID: %d - Error SEG_FAULT - Segmento: %d - Offset: %d - Tamaño: %d", pcb->pid, datos_mmu.numero_segmento, datos_mmu.desplazamiento_segmento, datos_mmu.tamanio_segmento); //log obligatorio

					enviar_pcb_a_kernel(pcb, EXIT_CON_SEG_FAULT, parametros_actuales);
					return;
				}
				else {
					//Atentos a la mayor (ya no... xd) villereada de todos los tiempos!!

				    // EJECUCION INSTRUCCION
					switch(instruccion_actual) {
						case MOV_IN:
							ejecutar_mov_in(pcb, datos_mmu, obtener_registro(instruccion_actual, parametros_actuales));
							break;
						case MOV_OUT:
							ejecutar_mov_out(pcb, datos_mmu, obtener_registro(instruccion_actual, parametros_actuales));
							break;
						default:
							ejecutar_fread_o_fwrite(pcb, datos_mmu.direccion_fisica, instruccion_actual, parametros_actuales);
							return;
					}
				}

				break;

			case IO: case F_OPEN: case F_CLOSE: case F_SEEK: case F_TRUNCATE: case WAIT:
			case SIGNAL: case CREATE_SEGMENT: case DELETE_SEGMENT: case YIELD: case EXIT:
				enviar_pcb_a_kernel(pcb, instruccion_actual, parametros_actuales);
				return;

			default:
				log_error(my_logger, "Error en el decode de la instruccion");
				//exit(EXIT_FAILURE);
				break;
		}
	}
}

t_msj_kernel_cpu instruccion_a_enum(t_instruccion* instruccion) {
	char* nombre = instruccion->nombre;

	//Las que usan Memoria y CPU
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

	//Las que solo usa CPU:
	if(!strcmp(nombre, "SET")) 			  return SET;
	if(!strcmp(nombre, "MOV_IN")) 	      return MOV_IN;
	if(!strcmp(nombre, "MOV_OUT")) 		  return MOV_OUT;

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

	usleep(lectura_de_config.RETARDO_INSTRUCCION * 1000); //Porque me lo dan en milisegundos
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

	memcpy(valor + tamanio_registro(registro), "\0", sizeof(char)); //Para que pueda funcionar bien con las funciones de string_array

	//usleep(lectura_de_config.RETARDO_INSTRUCCION * 1000); //Porque me lo dan en milisegundos
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
	// Hay que hacer un free y no un string_array_destroy porque sino estraría liberando los del pcb
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
				return segmento->direccion_base;
			}
			if(!strcmp(campo, "tamanio")) {
				return segmento->tamanio;
			}
		}
	}

	return -1;
}

t_datos_mmu mmu(t_pcb* pcb, int direccion_logica) {
	t_datos_mmu datos;

	datos.numero_segmento = direccion_logica / lectura_de_config.TAM_MAX_SEGMENTO; //al asignarle a un int se obtiene el floor
	datos.desplazamiento_segmento = direccion_logica % lectura_de_config.TAM_MAX_SEGMENTO;

	datos.tamanio_segmento = buscar_campo_de_segmento(pcb->tabla_segmentos, "tamanio", datos.numero_segmento);

	datos.direccion_fisica = datos.desplazamiento_segmento + buscar_campo_de_segmento(pcb->tabla_segmentos, "base", datos.numero_segmento);

	return datos;
}

////////////////////////////////////////
// Funciones de abstracción de lógica //
////////////////////////////////////////

int numero_de_parametro_de_direccion_logica(t_msj_kernel_cpu instruccion_actual) {
	return instruccion_actual != MOV_OUT;
	/* anda porque sí agus basta
	solamente lo hice en una función aparte para que se entienda más
	y por si (nunca va a pasar) en algún futuro se agregan más instrucciones y etc
	Pero imaginate, podría haber hecho directamente:
	int direccion_logica = list_get(instruccion_actual->parametros, instruccion_actual != MOV_OUT);
	*/
}

int obtener_cantidad_de_bytes(t_msj_kernel_cpu instruccion_actual, t_list* parametros) {
	if(instruccion_actual == F_READ || instruccion_actual == F_WRITE) {
		return atoi(list_get(parametros, 2));
	}
	else {
		return tamanio_registro(obtener_registro(instruccion_actual, parametros));
	}
}

char* obtener_registro(t_msj_kernel_cpu instruccion_actual, t_list* parametros) {
	return list_get(parametros, numero_de_parametro_de_registro(instruccion_actual));
}

int numero_de_parametro_de_registro(t_msj_kernel_cpu instruccion_actual) {
	return instruccion_actual == MOV_OUT; //De vuelta, anda pq sí pero basta, es sexo
}

/////////////////////////////////
//Funciones de ejecución posta //
/////////////////////////////////

void ejecutar_mov_in(t_pcb* pcb, t_datos_mmu datos_mmu, char* nombre_registro) {
	char** parametros = string_array_new();
	string_array_push(&parametros, string_itoa(datos_mmu.direccion_fisica));
	string_array_push(&parametros, string_itoa(tamanio_registro(nombre_registro)));
	string_array_push(&parametros, string_itoa(pcb->pid));

	enviar_msj_con_parametros(socket_memoria, LEER_VALOR, parametros);

	string_array_destroy(parametros);

	char ** parametros_lectura;

	if(recibir_msj(socket_memoria) == LEIDO_OK) { //No hace falta pero bueno, recibe un mensaje sí o sí
		parametros_lectura = recibir_parametros_de_mensaje(socket_memoria);

		char* valor = parametros_lectura[0];

		set_registro(pcb, nombre_registro, valor);

		log_info(logger, "PID: %d - Accion: LEER - Segmento: %d - Dirección Física: %d - Valor: %s", pcb->pid, datos_mmu.numero_segmento, datos_mmu.direccion_fisica, valor); //log obligatorio
	}
	else {
		log_error(my_logger, "No se pudo leer en memoria");
		exit(EXIT_FAILURE);
	}

	string_array_destroy(parametros_lectura);
}

void ejecutar_mov_out(t_pcb* pcb, t_datos_mmu datos_mmu, char* nombre_registro) {
	char** parametros = string_array_new();
	string_array_push(&parametros, string_itoa(datos_mmu.direccion_fisica));
	char* valor = leer_registro(pcb, nombre_registro);
	string_array_push(&parametros, valor);
	string_array_push(&parametros, string_itoa(pcb->pid));

	enviar_msj_con_parametros(socket_memoria, ESCRIBIR_VALOR, parametros);

	if(recibir_msj(socket_memoria) == ESCRITO_OK) { //No hace falta pero bueno, recibe un mensaje sí o sí
		log_info(logger, "PID: %d - Acción: ESCRIBIR - Segmento: %d - Dirección Física: %d - Valor: %s", pcb->pid, datos_mmu.numero_segmento, datos_mmu.direccion_fisica, valor); //log obligatorio
	}
	else {
		log_error(my_logger, "No se pudo escribir en memoria");
		exit(EXIT_FAILURE);
	}

	string_array_destroy(parametros);
}

void ejecutar_fread_o_fwrite(t_pcb* pcb, int direccion_fisica, t_msj_kernel_cpu instruccion_actual, t_list* parametros) {
	t_list* parametros_a_enviar = list_create();

	char* str_direccion_fisica = string_itoa(direccion_fisica);

	list_add(parametros_a_enviar, list_get(parametros, 0)); //Nombre del archivo
	list_add(parametros_a_enviar, str_direccion_fisica);
	list_add(parametros_a_enviar, list_get(parametros, 2)); //Cantidad de bytes

	enviar_pcb_a_kernel(pcb, instruccion_actual, parametros_a_enviar);

	list_destroy(parametros_a_enviar);
	free(str_direccion_fisica);
}
