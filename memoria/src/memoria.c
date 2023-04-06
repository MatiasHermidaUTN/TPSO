#include <stdio.h>
#include <stdlib.h>
#include <utils.h>

int main(void) {
	puts("!!!Hello World!!!"); /* prints !!!Hello World!!! */

    int server = iniciar_servidor("127.0.0.1","4444");
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

		return EXIT_SUCCESS;
}
