#include <stdio.h>
#include <stdlib.h>

#include <utils.h>
#include "../include/configuracion_cpu.h"
#include "../include/ciclo_instruccion.h"

int main(int argc, char** argv) {
    t_config* config = iniciar_config("../cpu.config");
    t_cpu_config lectura_de_config = leer_cpu_config(config);
    t_log* logger = iniciar_logger("cpu.config", "CPU");

    int socket_memoria = crear_conexion(lectura_de_config.IP_MEMORIA, lectura_de_config.PUERTO_MEMORIA);
    enviar_handshake(socket_memoria, CPU);

    t_handshake respuesta = recibir_handshake(socket_memoria);
    if(respuesta == ERROR) {
        log_error(logger, "La CPU no se pudo conectar a la Memoria");
        exit(EXIT_FAILURE);
    }

    int socket_cpu = iniciar_servidor("127.0.0.1", lectura_de_config.PUERTO_ESCUCHA);
    puts("CPU listo para recibir al Kernel");
    int socket_kernel = esperar_cliente(socket_cpu);

    liberar_estructura_config(lectura_de_config);

    puts("El Kernel se conecto a la CPU");

    while (1) {
    	int cod_op = recibir_operacion(socket_kernel);
    	switch (cod_op) {
    		case MENSAJE:
    			recibir_mensaje(socket_kernel);
    			break;
    		case PCB:
    			correr_ciclo_instruccion(socket_kernel);
    			break;
        }
        return 0;
    }

	return EXIT_SUCCESS;
}
