#include <commons/log.h>

#include <utils.h>
#include "../include/configuracion_kernel.h"
#include "../include/conexiones_kernel.h"
#include "../include/planificador.h"

int main (int argc, char** argv) {
    //t_log* logger = iniciar_logger();

    t_config* config = iniciar_config(argv[1]);
    t_kernel_config lectura_de_config = leer_kernel_config(config);
    t_log* logger = iniciar_logger("kernel.log", "proceso");

    int socket_memoria;
    int socket_cpu;
    int socket_fileSystem;
    inicializar_conexiones(lectura_de_config, logger, &socket_memoria, &socket_cpu, &socket_fileSystem);

    int socket_kernel = iniciar_servidor("127.0.0.1", lectura_de_config.PUERTO_ESCUCHA); //TODO: Hardcodeado
    while(recibir_conexiones(socket_kernel, logger, lectura_de_config.ESTIMACION_INICIAL)); //Recibe conexiones y crea hilos para manejarlas

    config_destroy(config);
    liberar_estructura_de_config(lectura_de_config);
    log_destroy(logger);
    close(socket_cpu);
    close(socket_fileSystem);
    close(socket_memoria);

    return 0;
}
