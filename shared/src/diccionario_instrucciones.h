#ifndef DICCIONARIO_INSTRUCCIONES_H_
#define DICCIONARIO_INSTRUCCIONES_H_

#include "utils.h"
#include <commons/collections/dictionary.h>

t_dictionary* crear_diccionario_instrucciones();
void agregar_a_diccionario(t_dictionary* diccionario, char* key, int elemento);

#endif /* DICCIONARIO_INSTRUCCIONES_H_ */
