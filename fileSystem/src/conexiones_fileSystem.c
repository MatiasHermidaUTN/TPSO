#include "../include/conexiones_fileSystem.h"

void init_conexiones() {
	//SE CONECTA AL SERIVDOR MEMORIA
	socket_memoria = crear_conexion(lectura_de_config.IP_MEMORIA, lectura_de_config.PUERTO_MEMORIA);
	if(socket_memoria == -1){
		printf("No conectado a memoria\n");
	}
    enviar_handshake(socket_memoria, FILESYSTEM);
    int rta = recibir_handshake(socket_memoria);
    if(rta != OK_HANDSHAKE){
    	log_error(logger,"error en el handshake");
    	exit(EXIT_FAILURE);
    }

	//SE HACE SERVIDOR Y ESPERA LA CONEXION DEL KERNEL
	int server = iniciar_servidor("127.0.0.1", lectura_de_config.PUERTO_ESCUCHA);
	log_warning(logger,"Servidor listo para recibir al cliente, puerto: %s", lectura_de_config.PUERTO_ESCUCHA);
	kernel = esperar_cliente(server);
	log_warning(logger,"Se conecto el Kernel a FileSystem");

	pthread_t hilo_escuchar_kernel;
    pthread_create(&hilo_escuchar_kernel, NULL, (void*)escuchar_kernel, NULL);
    pthread_detach(hilo_escuchar_kernel);
}
