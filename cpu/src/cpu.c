#include <stdio.h>
#include <stdlib.h>

#include "../../shared/include/utils.h"
#include "../include/configuracion_cpu.h"


int main(int argc, char** argv) {
    t_config* config = iniciar_config("../cpu.config");

    t_cpu_config lectura_de_config = leer_cpu_config(config);

    int socket_memoria = crear_conexion(lectura_de_config.IP_MEMORIA, lectura_de_config.PUERTO_MEMORIA);
    enviar_handshake(socket_memoria, CPU);


    int server = iniciar_servidor("127.0.0.1",lectura_de_config.PUERTO_ESCUCHA);
    puts("Servidor listo para recibir al cliente");
    int cliente = esperar_cliente(server);

    puts("Se conecto el Kernel a la CPU");

    while (1) {
    	int cod_op = recibir_operacion(cliente);
    	switch (cod_op) {
    	case MENSAJE:
    		recibir_mensaje(cliente);
    		break;
            }
        return 0;
    }

	return EXIT_SUCCESS;
}