#include <stdio.h>
#include <stdlib.h>

#include <utils.h>
#include "../include/configuracion_fileSystem.h"

int main(int argc, char** argv) {
	t_config* config = iniciar_config(argv[1]);
    t_fileSystem_config lectura_de_config = leer_fileSystem_config(config);

	int socket_memoria = crear_conexion(lectura_de_config.IP_MEMORIA, lectura_de_config.PUERTO_MEMORIA);
    enviar_handshake(socket_memoria, FILESYSTEM);

    int socket_fileSystem = iniciar_servidor("127.0.0.1", lectura_de_config.PUERTO_ESCUCHA);

    puts("File System listo para recibir al Kernel");
	int socket_kernel = esperar_cliente(socket_fileSystem);
	puts("El Kernel se conecto a File System");

	while (1) {
		int cod_op = recibir_operacion(socket_kernel);
		switch (cod_op) {
			case MENSAJE:
				recibir_mensaje(socket_kernel);
				break;
	    }
	    return 0;
	}

	config_destroy(config);
	liberar_estructura_de_config(lectura_de_config);
	close(socket_memoria);

	return EXIT_SUCCESS;
}
