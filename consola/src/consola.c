#include <stdio.h>
#include <commons/string.h>
#include <commons/config.h>
#include <commons/log.h>
#include <commons/collections/dictionary.h>
#include <commons/collections/list.h>

#include <utils.h>
#include "../include/configuracion_consola.h" // Fijarse si está bien
#include "../include/parser.h"
#include "../include/comunicaciones_consola.h"

void liberar_memoria(t_list* instrucciones, t_log* logger, t_config* config, t_consola_config lectura_de_config, int socket_kernel);

int main(int argc, char** argv) {
	// ***** INICIAR CONSOLA ***** //
    t_config* config = iniciar_config(argv[1]);
    t_log* logger    = iniciar_logger("consola.log", "proceso");
    t_consola_config lectura_de_config = leer_consola_config(config);

	log_info(logger, "IP KERNEL: %s", lectura_de_config.IP_KERNEL); //el %s es para que no tire warning para tomarlo como literal cadena
	log_info(logger, "Puerto KERNEL: %s", lectura_de_config.PUERTO_KERNEL);

    // ***** PARSEAR INSTRUCCIONES ***** //
    t_list* instrucciones = parsearPseudocodigo(logger, argv[2]);
    //log_warning(logger, "Cantidad de bytes a mandar: %d", calculo_tamanio_msj(instrucciones));

    // ***** CONECTAR A KERNEL ***** //
	int socket_kernel = crear_conexion(lectura_de_config.IP_KERNEL, lectura_de_config.PUERTO_KERNEL);

	// ***** ENVIAR INSTRUCCIONES A KERNEL ***** //
	enviar_instrucciones(socket_kernel, instrucciones);
	//esperar_confirmacion(socket_kernel);
	//esperar_fin_proceso(socket_kernel);

	// ***** LIBERAR MEMORIA Y CERRAR ***** //
	liberar_memoria(instrucciones, logger, config, lectura_de_config, socket_kernel);

    return 0;
}

void liberar_memoria(t_list* instrucciones, t_log* logger, t_config* config, t_consola_config lectura_de_config, int socket_kernel) {
	list_destroy_and_destroy_elements(instrucciones, (void*)destruir_instruccion);
	log_destroy(logger);
	config_destroy(config);
	liberar_estructura_de_config(lectura_de_config);
	close(socket_kernel);
}
