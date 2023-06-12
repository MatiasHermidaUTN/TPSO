#include "../include/conexiones_fileSystem.h"

void init_conexiones() {
	//SE CONECTA AL SERIVDOR MEMORIA
	socket_memoria = crear_conexion(lectura_de_config.IP_MEMORIA, lectura_de_config.PUERTO_MEMORIA);
	if(socket_memoria == -1){
		printf("No conectado a memoria\n");
	}
    enviar_handshake(socket_memoria, FILESYSTEM);

	//SE HACE SERVIDOR Y ESPERA LA CONEXION DEL KERNEL
	int server = iniciar_servidor("127.0.0.1", lectura_de_config.PUERTO_ESCUCHA);
	printf("Servidor listo para recibir al cliente, puerto: %s\n", lectura_de_config.PUERTO_ESCUCHA);
	kernel = esperar_cliente(server);
	puts("Se conecto el Kernel a FileSystem\n");

	pthread_t hilo_escuchar_kernel;
    pthread_create(&hilo_escuchar_kernel, NULL, (void*)escuchar_kernel, NULL);
    pthread_detach(hilo_escuchar_kernel);
}
