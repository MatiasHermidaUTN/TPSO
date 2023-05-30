#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include <math.h>
#include <time.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <commons/bitarray.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>
#include <utils.h>

#include "../include/manejar_mensajes.h"
#include "../include/conexiones_fileSystem.h"
#include "../include/fileSystem.h"
#include "../include/configuracion_fileSystem.h"

int main(int argc, char** argv) {

	//LECTURA DE CONFIG DEL FILESYSTEM
	iniciar_logger("FileSystem.log", "FS");
	iniciar_config("../fileSystem.config");
    lectura_de_config = leer_fileSystem_config(config);

	lista_fifo_msj = list_create();

	sem_init(&sem_sincro_cant_msj, 0, 0); 		//msj es tarea a ejecutar(escribir, leer, etc.)
	pthread_mutex_init(&mutex_cola_msj, NULL);

	//SUPERBLOQUE, BITMAP, BLOQUES, carpeta FCBs
	init_superbloque();
	init_bitmap();
	init_bloques();
	init_carpeta_fcbs();

	init_conexiones();

	while(manejar_mensaje());

	log_destroy(logger);
    config_destroy(config);
	config_destroy(superbloque);
	msync(bitmap_pointer, tamanioBitmap, MS_SYNC);
	munmap(bitmap_pointer, tamanioBitmap);
	fclose(bloques);
	return EXIT_SUCCESS;
}
