#include <commons/log.h>
#include <utils.h>
#include "../include/configuracion_kernel.h"

int main (int argc, char** argv) {
    //t_log* logger = iniciar_logger();

    t_config* config = iniciar_config("kernel.config");

    t_kernel_config lectura_de_config = leer_kernel_config(config);

    int memoria    = crear_conexion(lectura_de_config.IP_MEMORIA, lectura_de_config.PUERTO_MEMORIA);
    int cpu        = crear_conexion(lectura_de_config.IP_CPU, lectura_de_config.PUERTO_CPU);
    int fileSystem = crear_conexion(lectura_de_config.IP_FILESYSTEM, lectura_de_config.PUERTO_FILESYSTEM);


    int server = iniciar_servidor("127.0.0.1","4444");
    //log_info(logger, "Servidor listo para recibir al cliente");
    puts("Servidor listo para recibir al cliente");
    int cliente = esperar_cliente(server);
    puts("se concento alguien");

    while (1) {
		int cod_op = recibir_operacion(cliente);
		switch (cod_op) {
		    case MENSAJE:
		    	recibir_mensaje(cliente);
		    	break;
        }
    }
    return 0;
}
