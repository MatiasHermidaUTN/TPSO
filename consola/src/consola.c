#include "../include/consola.h"

int main(int argc, char** argv) {
    t_config* config = iniciar_config(argv[1]);
    t_consola_config lectura_de_config = leer_consola_config(config);

    t_log* logger    = iniciar_logger("consola.log", "Consola");

	log_info(logger, "IP KERNEL: %s", lectura_de_config.IP_KERNEL);
	log_info(logger, "Puerto KERNEL: %s", lectura_de_config.PUERTO_KERNEL);


	int socket_kernel = crear_conexion(lectura_de_config.IP_KERNEL, lectura_de_config.PUERTO_KERNEL);

    t_list* instrucciones = parsearPseudocodigo(logger, argv[2]);

	enviar_instrucciones(socket_kernel, instrucciones);

	esperar_fin_proceso(socket_kernel, logger);

    list_destroy_and_destroy_elements(instrucciones, (void*)destruir_instruccion);
    log_destroy(logger);
    config_destroy(config);
    liberar_estructura_config(lectura_de_config);

    close(socket_kernel);
    return 0;
}

void esperar_fin_proceso(int socket_kernel, t_log* logger) {
	switch(recibir_msj(socket_kernel)) {
		case SUCCESS:
			log_info(logger, "Proceso terminado exitosamente");
			break;
		case OUT_OF_MEMORY:
			log_info(logger, "Proceso terminado por falta de memoria");
			break;
		case SEG_FAULT:
			log_info(logger, "Proceso terminado por Segmentation Fault");
			break;
		default:
			log_error(logger, "Hubo un error en el proceso");
			exit(EXIT_FAILURE);
	}
}
