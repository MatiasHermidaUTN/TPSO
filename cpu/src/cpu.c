#include <stdio.h>
#include <stdlib.h>
#include <utils.h>
#include "../include/configuracion_cpu.h"
#include "../include/ejecucion_instrucciones.h"


int main(int argc, char** argv) {
    t_config* config = iniciar_config(argv[1]);
    lectura_de_config = leer_cpu_config(config);

    logger = iniciar_logger("cpu.config", "CPU");

    int socket_memoria = crear_conexion(lectura_de_config.IP_MEMORIA, lectura_de_config.PUERTO_MEMORIA);
    enviar_handshake(socket_memoria, CPU);
    t_handshake respuesta = recibir_handshake(socket_memoria);
    if(respuesta == ERROR_HANDSHAKE){
        log_error(logger,"La CPU no se pudo conectar a la memoria");
        exit(EXIT_FAILURE);
    }

    int socket_cpu = iniciar_servidor("127.0.0.1",lectura_de_config.PUERTO_ESCUCHA);
    log_warning(logger, "CPU lista para recibir al Kernel");
    socket_kernel = esperar_cliente(socket_cpu);

    puts("Se conecto el Kernel a la CPU");

    while (1) {
    	int cod_op = recibir_operacion(socket_kernel);
    	switch (cod_op){
    		case PCB_A_EJECUTAR:
    			t_pcb* pcb = recibir_pcb(socket_kernel); //deserializar hace el malloc
    			log_warning(logger, "PID RECIBIDO: %d", pcb->pid);
    			ejecutar_instrucciones(pcb);
    			liberar_pcb(pcb);
    			break;
        }
    }

	return EXIT_SUCCESS;
}
