#include "../include/consola.h"

int main(int argc, char** argv) {
    config = iniciar_config(argv[1]);
    t_consola_config lectura_de_config = leer_consola_config();

    logger = iniciar_logger("consola.log", "Consola");

	log_warning(logger, "IP Kernel: %s", lectura_de_config.IP_KERNEL);
	log_warning(logger, "Puerto Kernel: %s", lectura_de_config.PUERTO_KERNEL);

	socket_kernel = crear_conexion(lectura_de_config.IP_KERNEL, lectura_de_config.PUERTO_KERNEL);

    t_list* instrucciones = parsear_pseudocodigo(argv[2]);

	enviar_instrucciones(instrucciones);

	esperar_fin_proceso();

    list_destroy_and_destroy_elements(instrucciones, (void*)destruir_instruccion);
    log_destroy(logger);
    config_destroy(config);
    liberar_estructura_config(lectura_de_config);
    close(socket_kernel);

    return 0;
}

void esperar_fin_proceso() {
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
