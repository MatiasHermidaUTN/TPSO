#include "diccionario_instrucciones.h"

t_dictionary* crear_diccionario_instrucciones() {
	t_dictionary* diccionario_instrucciones = dictionary_create();
	agregar_a_diccionario(diccionario_instrucciones, "F_READ",         3);
	agregar_a_diccionario(diccionario_instrucciones, "F_WRITE",        3);
	agregar_a_diccionario(diccionario_instrucciones, "SET",            2);
	agregar_a_diccionario(diccionario_instrucciones, "MOV_IN",         2);
	agregar_a_diccionario(diccionario_instrucciones, "MOV_OUT",        2);
	agregar_a_diccionario(diccionario_instrucciones, "F_TRUNCATE", 	   2);
	agregar_a_diccionario(diccionario_instrucciones, "F_SEEK",    	   2);
	agregar_a_diccionario(diccionario_instrucciones, "CREATE_SEGMENT", 2);
	agregar_a_diccionario(diccionario_instrucciones, "I/O",            1);
	agregar_a_diccionario(diccionario_instrucciones, "WAIT",           1);
	agregar_a_diccionario(diccionario_instrucciones, "SIGNAL",         1);
	agregar_a_diccionario(diccionario_instrucciones, "F_OPEN",         1);
	agregar_a_diccionario(diccionario_instrucciones, "F_CLOSE",        1);
	agregar_a_diccionario(diccionario_instrucciones, "DELETE_SEGMENT", 1);
	agregar_a_diccionario(diccionario_instrucciones, "EXIT",           0);
	agregar_a_diccionario(diccionario_instrucciones, "YIELD",          0);
	return diccionario_instrucciones;
}

void agregar_a_diccionario(t_dictionary* diccionario, char* key, int elemento) {
	int* elemento_a_agreagar = malloc(sizeof(elemento));
	*elemento_a_agreagar = elemento;
	dictionary_put(diccionario, key, elemento_a_agreagar);
}

void destruir_diccionario(int* cantParametros) {
	free(cantParametros);
}
