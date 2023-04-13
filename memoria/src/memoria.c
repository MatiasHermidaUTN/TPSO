#include <stdio.h>
#include <stdlib.h>
#include <utils.h>
#include <commons/log.h>
#include "../include/configuracion_memoria.h"
#include "../include/conexiones_memoria.h"

int main(int argc, char** argv) {
	t_config* config = iniciar_config("../memoria.config");

	t_memoria_config lectura_de_config = leer_memoria_config(config);

    t_log* logger = iniciar_logger("memoria.log", "proceso");


    int socket_memoria = iniciar_servidor("127.0.0.1", lectura_de_config.PUERTO_ESCUCHA); //preguntar si la ip tiene que estar en el config o no
	puts("Servidor listo para recibir al cliente");

	while(recibir_conexiones(socket_memoria, logger)); //Recibe conexiones y crea hilos para manejarlas

	return EXIT_SUCCESS;

}
