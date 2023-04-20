#include <commons/log.h>
#include <utils.h>
#include "../include/conexiones_kernel.h"
#include "../include/kernel.h"
#include "../include/configuracion_kernel.h"

int main (int argc, char** argv) {
    t_config* config = iniciar_config("../kernel.config");
    lectura_de_config = leer_kernel_config(config);

    logger = iniciar_logger("kernel.log", "proceso");

    init_semaforos();
    init_estados();

    init_conexiones(lectura_de_config, logger, &socket_memoria, &socket_cpu, &socket_fileSystem);

    pthread_t hilo_planificador_corto;
	t_args_recibir_conexiones* args = malloc(sizeof(t_args_recibir_conexiones)); //tiene que ser puntero?
	args->socket_cliente = socket_cpu;
    pthread_create(&hilo_planificador_corto, NULL, (void*)planificar_corto, (void*)args);

    int socket_kernel = iniciar_servidor("127.0.0.1", lectura_de_config.PUERTO_ESCUCHA); //TODO: Hardcodeado
    while(recibir_conexiones(socket_kernel)); //Recibe conexiones de consolas y crea hilos para manejarlas

    liberar_estructura_config(lectura_de_config);
    config_destroy(config);
    return 0;
}
