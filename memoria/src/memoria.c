#include <stdio.h>
#include <stdlib.h>
#include <utils.h>
#include "../include/configuracion_memoria.h"

int main(int argc, char** argv) {
	t_config* config = iniciar_config("memoria.config");

	t_memoria_config lectura_de_config = leer_memoria_config(config);

    int server = iniciar_servidor(lectura_de_config.IP_MEMORIA, lectura_de_config.PUERTO_MEMORIA);
	puts("Servidor listo para recibir al cliente");


// Rever habiendo visto hilos
	int cpu		   = esperar_cliente(server);
	int fileSystem = esperar_cliente(server);
	int kernel	   = esperar_cliente(server);

    puts("se concento alguien");

    while (1) {
    	int cod_op = recibir_operacion(cpu);
    	switch (cod_op) {
    		case MENSAJE:
    			recibir_mensaje(cpu);
    			break;
        }
        return 0;
    }

	return EXIT_SUCCESS;

}
