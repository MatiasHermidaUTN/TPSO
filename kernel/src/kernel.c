#include <commons/log.h>
#include <utils.h>
#include "../include/configuracion_kernel.h"
#include "../include/conexiones_kernel.h"

int main (int argc, char** argv) {
    //t_log* logger = iniciar_logger();

    t_config* config = iniciar_config("../kernel.config");
    t_kernel_config lectura_de_config = leer_kernel_config(config);

    t_log* logger = iniciar_logger("kernel.log", "proceso");


    int socket_memoria = crear_conexion(lectura_de_config.IP_MEMORIA, lectura_de_config.PUERTO_MEMORIA);
    if(socket_memoria == -1){
    	log_error(logger, "El kernel no pudo conectarse a la memoria");
    	exit(EXIT_FAILURE);
    }
    enviar_handshake(socket_memoria, KERNEL);

    //Para estos no hace falta handshake porque solo reciben al Kernel
    int socket_cpu = crear_conexion(lectura_de_config.IP_CPU, lectura_de_config.PUERTO_CPU);
    if(socket_cpu == -1){
        	log_error(logger, "El kernel no pudo conectarse a el CPU");
        	exit(EXIT_FAILURE);
    }

    int socket_fileSystem = crear_conexion(lectura_de_config.IP_FILESYSTEM, lectura_de_config.PUERTO_FILESYSTEM);
    if(socket_fileSystem == -1){
            	log_error(logger, "El kernel no pudo conectarse a el FileSystem");
            	exit(EXIT_FAILURE);
    }


    int socket_kernel = iniciar_servidor("127.0.0.1",lectura_de_config.PUERTO_ESCUCHA); //TODO: Hardcodeado
    while(recibir_conexiones(socket_kernel, logger)); //Recibe conexiones y crea hilos para manejarlas

    return 0;
}
