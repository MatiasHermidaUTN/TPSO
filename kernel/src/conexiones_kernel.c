#include "../include/conexiones_kernel.h"
#include "../include/pcb_kernel.h"
#include "../include/planificador.h"

int recibir_conexiones(int socket_kernel, t_log* logger, int config_estimacion_inicial) {
	log_info(logger, "\nKernel listo para recibir Consola");
	int socket_consola = esperar_cliente(socket_kernel);
	if(socket_consola != -1) { //TODO: checkear -1 es error
	    t_queue* new = queue_create();
	    //inicializar_estados(new, ready, execute, block, exit);

		pthread_t hilo;
		t_args_recibir_conexiones* args = malloc(sizeof(t_args_recibir_conexiones)); //tiene que ser puntero?
		args->socket_cliente = socket_consola;
		args->logger = logger;
		args->config_estimado_de_proxima_rafaga = config_estimacion_inicial;
		args->new = new;

		pthread_create(&hilo, NULL, (void*)manejar_conexion, (void*)args);
		pthread_detach(hilo);

		return 1;
	}
	return 0;
}

void manejar_conexion(void* args) {
	int socket_consola = ((t_args_recibir_conexiones*)args)->socket_cliente;
	t_log* logger = ((t_args_recibir_conexiones*)args)->logger;
	int config_estimado_de_proxima_rafaga = ((t_args_recibir_conexiones*)args)->config_estimado_de_proxima_rafaga;
	t_queue* new = ((t_args_recibir_conexiones*)args)->new;
	free(args);

	log_info(logger, "Se conecto una consola");

	//TODO: Implementar
	op_code cod_op = recibir_operacion(socket_consola);
	switch(cod_op) {
		case NUEVO_PROCESO:
			//log_info(logger, "Consola mandando instrucciones");

			//recibir memsaje y deserializarlo
			t_list* proceso = recibir_proceso(socket_consola);
			//enviar_confirmacion_recepcion(socket_consola);

			// Inicializar PCB y enviar a CPU
			t_pcb* pcb = generar_pcb(proceso, config_estimado_de_proxima_rafaga);
			queue_push(new, pcb);
			log_info(logger, "Se crea el proceso %d en NEW", pcb->pid); // log obligatorio

			//**prueba con consola**
			print_l_instrucciones(pcb->instrucciones);
			//sleep(10);

			//enviar_fin_proceso(socket_consola);
			break;
		default:
			log_error(logger, "Error en el handshake");
	}
	//Liberar proceso
}

void inicializar_conexiones(t_kernel_config lectura_de_config, t_log* logger, int* socket_memoria, int* socket_cpu, int* socket_fileSystem) {
	*socket_memoria = crear_conexion(lectura_de_config.IP_MEMORIA, lectura_de_config.PUERTO_MEMORIA);
	if(*socket_memoria == -1) {
		log_error(logger, "El Kernel no pudo conectarse a la Memoria");
		exit(EXIT_FAILURE);
	}
	enviar_handshake(*socket_memoria, KERNEL);

	//Para estos no hace falta handshake porque solo reciben al Kernel
	*socket_cpu = crear_conexion(lectura_de_config.IP_CPU, lectura_de_config.PUERTO_CPU);
	if(*socket_cpu == -1) {
		log_error(logger, "El Kernel no pudo conectarse al CPU");
		exit(EXIT_FAILURE);
	}

	*socket_fileSystem = crear_conexion(lectura_de_config.IP_FILESYSTEM, lectura_de_config.PUERTO_FILESYSTEM);
	if(*socket_fileSystem == -1) {
		log_error(logger, "El Kernel no pudo conectarse al File System");
		exit(EXIT_FAILURE);
	}
}

