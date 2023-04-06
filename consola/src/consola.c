#include <utils.h>


int main(void)
{
	//#####################################################
	char* ip = "127.0.0.1";
    char* puerto = "4444";
	//#####################################################
	//#####################################################

	//logger = iniciar_logger();

	//log_info(logger, "Soy un Log");

	//config = iniciar_config();

	//if (config == NULL) {
	//	log_error(logger, "¡No se pudo crear el config!");
	//	terminar_programa_sin_conexion(logger, config);
	//}

	int conexion = crear_conexion(ip, puerto);

//
////// Enviamos al servidor el valor de CLAVE como mensaje
    puts("listosjhjdf");
    enviar_mensaje("jdsfkanfdsjkf", conexion);
    puts("listo");
	//terminar_programa(conexion, logger, config);
}


