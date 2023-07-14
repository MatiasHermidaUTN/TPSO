#include <commons/log.h>
#include <utils.h>
#include "../include/conexiones_kernel.h"
#include "../include/kernel.h"
#include "../include/configuracion_kernel.h"
#include "../include/escuchador_de_filesystem.h"

int main (int argc, char** argv) {
    t_config* config = iniciar_config(argv[1]);
    lectura_de_config = leer_kernel_config(config);

    logger = iniciar_logger("kernel.log", "Kernel");
    my_logger = iniciar_logger("my_kernel.log", "Kernel");

    init_estados();
    init_semaforos();
    init_conexiones();

    pthread_t planificador_corto;
    pthread_create(&planificador_corto, NULL, (void*)planificar_corto, NULL);
    pthread_detach(planificador_corto);

    pthread_t planificador_largo;
    pthread_create(&planificador_largo, NULL, (void*)planificar_largo, NULL);
    pthread_detach(planificador_largo);

    pthread_t escuchador_de_filesystem;
	pthread_create(&escuchador_de_filesystem, NULL, (void*)escuchar_de_filesystem, NULL);
	pthread_detach(escuchador_de_filesystem);

    int socket_kernel = iniciar_servidor(lectura_de_config.IP_KERNEL, lectura_de_config.PUERTO_ESCUCHA); //TODO: Hardcodeado
    log_warning(my_logger, "Kernel listo para recibir a Consolas");
    while(recibir_conexiones(socket_kernel)); //Recibe conexiones de consolas y crea hilos para manejarlas

    liberar_estructura_config(lectura_de_config);
    config_destroy(config);
    log_error(my_logger, "Fallaron las conexiones entre el Kernel y la Consola");
    exit(EXIT_FAILURE);
}
