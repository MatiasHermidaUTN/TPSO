#include <commons/log.h>
#include <utils.h>

int main (void){
    //t_log* logger = iniciar_logger();

    int server = iniciar_servidor();
    //log_info(logger, "Servidor listo para recibir al cliente");
    puts("Servidor listo para recibir al cliente");
    int cliente = esperar_cliente(server);
    puts("se concento alguien");

    while (1) {
		int cod_op = recibir_operacion(cliente);
		switch (cod_op) {
		case MENSAJE:
			recibir_mensaje(cliente);
			break;
            }
        return 0;
    }
}
