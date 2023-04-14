#include <commons/log.h>
#include <utils.h>
#include "../include/configuracion_kernel.h"
#include "../include/conexiones_kernel.h"

int main (int argc, char** argv) {
    //t_log* logger = iniciar_logger();

    t_config* config = iniciar_config("../kernel.config");
    t_kernel_config lectura_de_config = leer_kernel_config(config);

    t_log* logger = iniciar_logger("kernel.log", "proceso");

    int socket_memoria;
    int socket_cpu;
    int socket_fileSystem;
    init_conexiones(lectura_de_config, logger, &socket_memoria, &socket_cpu, &socket_fileSystem);

    int socket_kernel = iniciar_servidor("127.0.0.1",lectura_de_config.PUERTO_ESCUCHA); //TODO: Hardcodeado
    while(recibir_conexiones(socket_kernel, logger)); //Recibe conexiones y crea hilos para manejarlas

    return 0;
}
