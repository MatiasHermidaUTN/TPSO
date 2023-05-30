#include "../include/conexiones_fileSystem.h"

void init_conexiones() {
	//SE CONECTA AL SERIVDOR MEMORIA
	socket_memoria = crear_conexion(lectura_de_config.IP_MEMORIA, lectura_de_config.PUERTO_MEMORIA);
	if(socket_memoria == -1) {
		log_error(logger, "No se pudo conectar con Memoria");
		exit(EXIT_FAILURE);
	}
    enviar_handshake(socket_memoria, FILESYSTEM);

	//SE HACE SERVIDOR Y ESPERA LA CONEXION DEL KERNEL
	int server = iniciar_servidor("127.0.0.1", lectura_de_config.PUERTO_ESCUCHA);
	log_info(logger, "Servidor listo para recibir al cliente, puerto: %s\n", lectura_de_config.PUERTO_ESCUCHA);

	socket_kernel = esperar_cliente(server);
	if(socket_kernel == -1) {
		log_error(logger, "No se pudo conectar con Kernel");
		exit(EXIT_FAILURE);
	}
	log_info(logger, "Se conecto el Kernel a File System\n");

	pthread_t hilo_escuchar_kernel;
    pthread_create(&hilo_escuchar_kernel, NULL, (void*)escuchar_kernel, NULL);
    pthread_detach(hilo_escuchar_kernel);
}
