#include "../include/comunicaciones_kernel.h"

t_list* recibir_instrucciones(int socket_consola) {
	size_t size_payload;
    if(recv(socket_consola, &size_payload, sizeof(size_t), 0) != sizeof(size_t)) {
        return false;
    }

	void* payload = malloc(size_payload);

    if(recv(socket_consola, payload, size_payload, 0) != size_payload) {
        free(payload);
        exit(EXIT_FAILURE);
    }

    int desplazamiento = 0; //Hardcodeado nashe
    t_list* instrucciones = deserializar_instrucciones(payload, size_payload, &desplazamiento);

	free(payload);
	return instrucciones;
}

t_msj_kernel_cpu esperar_cpu() {
	t_msj_kernel_cpu respuesta;
	recv(socket_cpu, &respuesta, sizeof(t_msj_kernel_cpu), MSG_WAITALL);
	return respuesta;
}

char** recibir_parametros_de_instruccion() {
	size_t cantidad_de_parametros;
	recv(socket_cpu, &cantidad_de_parametros, sizeof(size_t), MSG_WAITALL);
	char** parametros = string_array_new();

	size_t tamanio_parametro;
	char* parametro_auxiliar;

	for(int i = 0; i < cantidad_de_parametros; i++) {
		recv(socket_cpu, &tamanio_parametro, sizeof(size_t), MSG_WAITALL);

		parametro_auxiliar = malloc(tamanio_parametro); //se libera cuando se hace string_array_destroy
		recv(socket_cpu, parametro_auxiliar, tamanio_parametro, MSG_WAITALL);

		string_array_push(&parametros, parametro_auxiliar);
	}

	return parametros; //acordarse de hacerle el free del otro lado
}


