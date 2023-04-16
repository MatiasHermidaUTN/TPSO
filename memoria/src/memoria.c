#include <stdio.h>
#include <stdlib.h>
#include <commons/log.h>

#include <utils.h>
#include "../include/configuracion_memoria.h"
#include "../include/conexiones_memoria.h"
#include "../include/comunicaciones_memoria.h"

int main(int argc, char** argv) {
	t_config* config = iniciar_config(argv[1]);
	t_memoria_config lectura_de_config = leer_memoria_config(config);
    t_log* logger = iniciar_logger("memoria.log", "proceso");

    int socket_memoria = iniciar_servidor("127.0.0.1", lectura_de_config.PUERTO_ESCUCHA); //preguntar si la ip tiene que estar en el config o no
	puts("Memoria lista para recibir cliente");

	while(recibir_conexiones(socket_memoria, logger)); //Recibe conexiones y crea hilos para manejarlas

	config_destroy(config);
	liberar_estructura_de_config(lectura_de_config);
	log_destroy(logger);

	return EXIT_SUCCESS;

}
