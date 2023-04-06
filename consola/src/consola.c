#include <utils.h>
#include <commons/string.h>
#include<commons/config.h>
#include<commons/log.h>


int main(int argc, char** argv)
{
    t_config* config = iniciar_config("consola.config");

    char* IP_KERNEL = config_get_string_value(config,"IP_KERNEL");
    char* PUERTO_KERNEL = config_get_string_value(config,"PUERTO_KERNEL");

    t_log* logger = iniciar_logger("consola.log","proceso");

	log_info(logger,"%s", IP_KERNEL);//el %s es para que no tire warning para tomarlo como literal cadena
	log_info(logger,"%s", PUERTO_KERNEL);



	int conexion = crear_conexion(IP_KERNEL, PUERTO_KERNEL);

//
////// Enviamos al servidor el valor de CLAVE como mensaje
    puts("antes de mandar");
    enviar_mensaje("mensaje1", conexion);
    enviar_mensaje("mensaje2", conexion);

    puts("despues de mandar");
	//terminar_programa(conexion, logger, config);
}


