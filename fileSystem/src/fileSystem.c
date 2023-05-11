#include "../include/fileSystem.h"
#define PRIMER_BLOQUE_SECUNDARIO 1
#define DESDE_EL_INICIO 0

t_config* superbloque;
//FILE* bitmap;
void* bitmap;
void* bitmap_pointer;
t_bitarray* bitarray_de_bitmap;
FILE* bloques;
//t_config* directorio_FCB;
int tamanioBitmap;

t_log* logger;
t_config* config;
t_fileSystem_config lectura_de_config;

int kernel;
int socket_memoria;

//pthread_mutex_t mutex_bloques;
//pthread_mutex_t mutex_bitmap;
//pthread_mutex_t mutex_FCB_default;

pthread_mutex_t mutex_cola_msj;
sem_t sem_sincro_cant_msj;

t_list* lista_fifo_msj;

int main(int argc, char** argv) {

	//pthread_mutex_init(&mutex_bloques, NULL);
	//pthread_mutex_init(&mutex_bitmap, NULL);
	//pthread_mutex_init(&mutex_FCB_default, NULL);

	//LECTURA DE CONFIG DEL FILESYSTEM
	logger = iniciar_logger("FileSystem.log", "FS");
	config = iniciar_config("../fileSystem.config");
    lectura_de_config = leer_fileSystem_config(config);

	lista_fifo_msj = list_create();

	sem_init(&sem_sincro_cant_msj, 0, 0); 		//msj == tarea a ejecutar(escribir, leer, etc.)
	pthread_mutex_init(&mutex_cola_msj, NULL);

	//CHQUEO DE QUE LOS PATHS A LOS DIFERENTES ARCHIVOS EXISTEN(SUPERBLOQUE, DIRECTORIO_FCB, BITMAP, BLOQUES)
	//SUPERBLOQUE
	if (archivo_se_puede_leer(lectura_de_config.PATH_SUPERBLOQUE)){
		superbloque = iniciar_config(lectura_de_config.PATH_SUPERBLOQUE);
		super_bloque_info.block_size = config_get_int_value(superbloque, "BLOCK_SIZE");
		super_bloque_info.block_count = config_get_int_value(superbloque, "BLOCK_COUNT");
		log_info(logger, "SuperBloque leido");
	} else {
		log_error(logger, "SuperBloque no esiste");
		//destroy everything in reality
		return EXIT_FAILURE;
	}

	//BITMAP
	tamanioBitmap = (int) ceil( (double) super_bloque_info.block_count/8.0 );
	int fd_bitmap = open(lectura_de_config.PATH_BITMAP, O_RDWR, 0664);	//abre o crea el archivo en un file descriptor
	if (fd_bitmap == -1) {
		close(fd_bitmap);
		log_warning(logger, "Creando bitmap");
		fd_bitmap = open(lectura_de_config.PATH_BITMAP, O_CREAT | O_RDWR, 0664); //abre o crea el archivo en un file descriptor
		if (fd_bitmap == -1){
			log_error(logger, "No se pudo ni CREAR el bitmap");
			//destroy everything in reality
			return EXIT_FAILURE;
		}
		ftruncate(fd_bitmap, tamanioBitmap+10);
	}
	bitmap_pointer = mmap(NULL, tamanioBitmap, PROT_READ | PROT_WRITE, MAP_SHARED, fd_bitmap, 0);
	bitarray_de_bitmap = bitarray_create_with_mode((char*) bitmap_pointer, tamanioBitmap, MSB_FIRST);
	close(fd_bitmap);
	log_info(logger, "Bitmap abierto");

	//BLOQUES
	bloques = fopen(lectura_de_config.PATH_BLOQUES, "r+");
	if (bloques == NULL) {
		log_warning(logger, "CREANDO BLOQUES");
		int fd_bloques = open(lectura_de_config.PATH_BLOQUES, O_CREAT | O_RDWR, 0664);	//abre o crea el archivo en un file descriptor
		if (fd_bloques == -1){
			log_error(logger, "No se pudo ni CREAR el bloques");
			//destroy everything in reality
			return EXIT_FAILURE;
		}
		int tamanioBloques = super_bloque_info.block_size * super_bloque_info.block_count;
		ftruncate(fd_bloques, tamanioBloques+10);
		close(fd_bloques);
		bloques = fopen(lectura_de_config.PATH_BLOQUES, "r+");
	}
	log_info(logger, "Bloques abierto");

	//FCBs directorio
	char* comando = malloc(10 + strlen("mkdir -v -p ") + strlen(lectura_de_config.PATH_FCB));
	strcpy(comando, "mkdir -v -p ");
	strcat(comando, lectura_de_config.PATH_FCB);
	system(comando);
	free(comando);

	//**************TODO DEBUG**************
	printf("unos inicial: %d\n", cant_unos_en_bitmap());
	//**************TODO DEBUG**************

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
	if(kernel == -1){
		return -1;
	}
	puts("Se conecto el Kernel a FileSystem\n");

	pthread_t hilo_escuchar_kernel;
    pthread_create(&hilo_escuchar_kernel, NULL, (void*)escuchar_kernel, NULL);
    pthread_detach(hilo_escuchar_kernel);


	while(manejar_mensaje());

	log_destroy(logger);
    config_destroy(config);
	config_destroy(superbloque);
	msync(bitmap_pointer, tamanioBitmap, MS_SYNC);
	munmap(bitmap_pointer, tamanioBitmap);
	fclose(bloques);
	return EXIT_SUCCESS;
}

void escuchar_kernel() {
	while(1){
		t_mensajes* args = malloc(sizeof(t_mensajes));
		args->cod_op = recibir_cod_op(kernel);
		printf("msj recibido\n");
		recibir_parametros(args->cod_op, &args->nombre_archivo, &args->nuevo_tamanio_archivo, &args->apartir_de_donde_X, &args->cuanto_X, &args->dir_fisica_memoria);
		list_push_con_mutex(lista_fifo_msj, args, &mutex_cola_msj);
		sem_post(&sem_sincro_cant_msj);
	}
	return;
}

int manejar_mensaje(){
	printf("esperando msj\n");
	sem_wait(&sem_sincro_cant_msj);

	printf("manejando msj\n");

	t_mensajes* args = list_pop_con_mutex(lista_fifo_msj, &mutex_cola_msj);

	t_instrucciones cod_op = ((t_mensajes*)args)->cod_op;
	char* nombre_archivo = strdup(((t_mensajes*)args)->nombre_archivo);
	int nuevo_tamanio_archivo = ((t_mensajes*)args)->nuevo_tamanio_archivo;
	int apartir_de_donde_X = ((t_mensajes*)args)->apartir_de_donde_X;
	int cuanto_X = ((t_mensajes*)args)->cuanto_X;
	int dir_fisica_memoria = ((t_mensajes*)args)->dir_fisica_memoria;
	free(((t_mensajes*)args)->nombre_archivo);
	free(args);

	char* buffer;

	switch (cod_op) {
		case ABRIR:{
			printf("abrir nombre_archivo: %s\n",nombre_archivo);
			if (existe_archivo(nombre_archivo)) {	//existe FCB?
				enviar_mensaje_kernel(kernel, "OK El archivo ya existe");
				printf("abierto %s\n",nombre_archivo);
			} else {
				enviar_mensaje_kernel(kernel, "ERROR El archivo NO existe");
				printf("no existe %s\n",nombre_archivo);
			}
			printf("\n");
			free(nombre_archivo);
			break;
		}
		case CREAR:{
			printf("crear nombre_archivo: %s \n", nombre_archivo);
			crear_archivo(nombre_archivo);	//crear FCB y poner tamaño 0 y sin bloques asociados.
			enviar_mensaje_kernel(kernel, "OK Archivo creado");
			printf("archivo creado: %s\n",nombre_archivo);
			printf("unos dsp crear: %d\n", cant_unos_en_bitmap());
			printf("\n");
			free(nombre_archivo);
			break;
		}
		case TRUNCAR:{
			printf("truncar nombre_archivo: %s\n", nombre_archivo);
			printf("nuevo_tamanio_archivo: %d\n", nuevo_tamanio_archivo);
			printf("unos antes truncar: %d\n", cant_unos_en_bitmap());
			truncar(nombre_archivo, nuevo_tamanio_archivo);
			enviar_mensaje_kernel(kernel, "OK Archivo truncado");
			printf("\n");
			free(nombre_archivo);
			break;
		}
		case LEER:{
			printf("leer nombre_archivo: %s\n", nombre_archivo);
			printf("apartir_de_donde_leer: %d\n", apartir_de_donde_X);
			printf("cuanto_leer: %d\n", cuanto_X);
			printf("dir_fisica_memoria: %d\n", dir_fisica_memoria);
			buffer = leer_archivo(nombre_archivo, apartir_de_donde_X, cuanto_X);	//malloc se hace en leer_archivo
			mandar_a_memoria(socket_memoria, ESCRIBIR, buffer, cuanto_X, dir_fisica_memoria);
			//esperar rta de memoria
			enviar_mensaje_kernel(kernel, "OK Archivo leido");
			printf("\n");
			printf("leido: ");
			for(int i = 0 ; i < cuanto_X ; i++){
				printf("%c", buffer[i]);
			}
			printf("\n");
			printf("\n");
			free(buffer);
			free(nombre_archivo);
			break;
		}
		case ESCRIBIR:{
			printf("escribir nombre_archivo: %s\n", nombre_archivo);
			printf("apartir_de_donde_escribir: %d\n", apartir_de_donde_X);
			printf("cuanto_escribir: %d\n", cuanto_X);
			printf("dir_fisica_memoria: %d\n", dir_fisica_memoria);
			buffer = leer_de_memoria(socket_memoria, LEER, cuanto_X, dir_fisica_memoria);	//malloc se hace en leer_de_memoria
			escribir_archivo(buffer, nombre_archivo, apartir_de_donde_X, cuanto_X);
			enviar_mensaje_kernel(kernel, "OK Archivo escrito");
			printf("\n");
			printf("escrito: %s\n", buffer);
			printf("\n");
			free(buffer);
			free(nombre_archivo);
			break;
		}
		case ERROR:
			//return 0; //?
			break;
		default:
			return 0;	//?
	}

	return 1;
}

void escribir_archivo(char* buffer, char* nombre_archivo, int apartir_de_donde_escribir, int cuanto_escribir) {	//llega del switch
	char* path = obtener_path_FCB_sin_free(nombre_archivo);
	t_config* archivo_FCB = iniciar_config(path);
	int tamanio_archivo = config_get_int_value(archivo_FCB, "TAMANIO_ARCHIVO");
	free(path);

	int cantidad_escrita = 0; //para ir recortando el buffer cuando ya leí
	//FAILSAFE
	if (apartir_de_donde_escribir > tamanio_archivo) {		//failsafe - no escribir donde no hay
		log_error(logger, "ERROR, apartir_de_donde_leer es > al tamanio_archivo");
		return;
	}
	if(apartir_de_donde_escribir + cuanto_escribir > tamanio_archivo){
		int nuevo_tamanio_archivo = apartir_de_donde_escribir + cuanto_escribir;
		truncar(nombre_archivo, nuevo_tamanio_archivo);
		printf("unos dsp truncar: %d\n", cant_unos_en_bitmap());
	}

	if (apartir_de_donde_escribir < super_bloque_info.block_size){
		uint32_t puntero_directo = config_get_int_value(archivo_FCB, "PUNTERO_DIRECTO");
		fseek(bloques, puntero_directo * super_bloque_info.block_size, SEEK_SET);

		escribir_bloque(buffer, puntero_directo, apartir_de_donde_escribir, &cuanto_escribir, &cantidad_escrita);
		sobreescribir_indirecto(buffer, archivo_FCB, PRIMER_BLOQUE_SECUNDARIO, &cuanto_escribir, &cantidad_escrita);
	}
	else {
		int bloque_secundario_inicial = (int) ceil((float) apartir_de_donde_escribir / super_bloque_info.block_size - 1);
		int apartir_de_donde_escribir_relativo_a_bloque = apartir_de_donde_escribir % super_bloque_info.block_size;
		apartir_de_donde_escribir_relativo_a_bloque==0? bloque_secundario_inicial++ : bloque_secundario_inicial;	//pq si apartir es 0, el bloque inicial va a dar el anterior

		uint32_t puntero_indirecto = config_get_int_value(archivo_FCB, "PUNTERO_INDIRECTO");
		uint32_t puntero_secundario = conseguir_ptr_secundario_para_indirecto(puntero_indirecto, bloque_secundario_inicial);

		escribir_bloque(buffer, puntero_secundario, apartir_de_donde_escribir_relativo_a_bloque, &cuanto_escribir, &cantidad_escrita);
		sobreescribir_indirecto(buffer, archivo_FCB, bloque_secundario_inicial + 1, &cuanto_escribir, &cantidad_escrita);
	}
	
	config_destroy(archivo_FCB);
	return;
}

void sobreescribir_indirecto(char* buffer, t_config* archivo_FCB, int bloque_secundario_donde_escribir, int* cuanto_escribir, int* cantidad_escrita) {
	uint32_t puntero_indirecto = config_get_int_value(archivo_FCB, "PUNTERO_INDIRECTO");
	uint32_t puntero_secundario;			

	while(*cuanto_escribir > 0) {
		puntero_secundario = conseguir_ptr_secundario_para_indirecto(puntero_indirecto, bloque_secundario_donde_escribir);
		escribir_bloque(buffer, puntero_secundario, DESDE_EL_INICIO, cuanto_escribir, cantidad_escrita);
		bloque_secundario_donde_escribir++;
	}
	return;
}

void escribir_bloque(char* buffer, uint32_t puntero, int apartir_de_donde_escribir_relativo_a_bloque, int* cuanto_escribir, int* cantidad_escrita) {
	usleep(atoi(lectura_de_config.RETARDO_ACCESO_BLOQUE) * 1000);
	char* copia_buffer = (char*) string_substring(buffer, *cantidad_escrita, *cuanto_escribir);
	//semaforo bloques
	//pthread_mutex_lock(&mutex_bloques);
	fseek(bloques, puntero * super_bloque_info.block_size + apartir_de_donde_escribir_relativo_a_bloque, SEEK_SET);

	if(apartir_de_donde_escribir_relativo_a_bloque + *cuanto_escribir > super_bloque_info.block_size){
		int dist_fondo_bloque_apartir_de_donde_esc = super_bloque_info.block_size - apartir_de_donde_escribir_relativo_a_bloque;
		fwrite(copia_buffer, dist_fondo_bloque_apartir_de_donde_esc, 1, bloques);
		*cantidad_escrita += dist_fondo_bloque_apartir_de_donde_esc;
		*cuanto_escribir -= dist_fondo_bloque_apartir_de_donde_esc;
	}
	else {
		fwrite(copia_buffer, *cuanto_escribir, 1, bloques);
		*cantidad_escrita += *cuanto_escribir;
		*cuanto_escribir = 0;
	}
	//pthread_mutex_unlock(&mutex_bloques);
	return;
}

void truncar(char* nombre_archivo, int nuevo_tamanio_archivo){
	char* path = obtener_path_FCB_sin_free(nombre_archivo);
	t_config* archivo_FCB = iniciar_config(path);

	int tamanio_archivo = config_get_int_value(archivo_FCB, "TAMANIO_ARCHIVO");

	if (nuevo_tamanio_archivo > tamanio_archivo){
		agrandas_archivo(archivo_FCB, nombre_archivo, nuevo_tamanio_archivo);
	}
	else if (nuevo_tamanio_archivo < tamanio_archivo) {
		achicas_archivo(archivo_FCB, nombre_archivo, nuevo_tamanio_archivo);
	}

	config_set_value(archivo_FCB, "TAMANIO_ARCHIVO", string_itoa(nuevo_tamanio_archivo));
	config_save_in_file(archivo_FCB, path);
	config_destroy(archivo_FCB);
	free(path);
	printf("\n");
	printf("unos dsp truncar: %d\n", cant_unos_en_bitmap());
	return;
}

uint32_t config_get_uint_value(t_config *self, char *key) {
	char *value = config_get_string_value(self, key);
	char *ptr;
	return (uint32_t) strtoul(value, &ptr, 10);
}

void achicas_archivo(t_config* archivo_FCB, char* nombre_archivo, int nuevo_tamanio_archivo){
	int tamanio_archivo = config_get_int_value(archivo_FCB, "TAMANIO_ARCHIVO");

	//entro si tiene puntero_directo y ya tiene puntero_indirecto
	if(tamanio_archivo > super_bloque_info.block_size){
		uint32_t puntero_indirecto = config_get_uint_value(archivo_FCB, "PUNTERO_INDIRECTO");

		int cant_punteros_secundarios = (int) ceil((float) tamanio_archivo / super_bloque_info.block_size - 1);
		int cant_bloques_secundarios_necesarios = (int) ceil((float) (nuevo_tamanio_archivo ? nuevo_tamanio_archivo : 1 ) / super_bloque_info.block_size - 1);
		if(cant_punteros_secundarios > cant_bloques_secundarios_necesarios){
			for( ; cant_punteros_secundarios > cant_bloques_secundarios_necesarios ; cant_punteros_secundarios--) {
				uint32_t puntero = conseguir_ptr_secundario_para_indirecto(puntero_indirecto, cant_punteros_secundarios);
				liberar_bloque(puntero);
			}
		}
		if(nuevo_tamanio_archivo <= super_bloque_info.block_size){
			liberar_bloque(puntero_indirecto);
			config_set_value(archivo_FCB, "PUNTERO_INDIRECTO", "");
		}
	}
	if(nuevo_tamanio_archivo == 0){
		uint32_t puntero_directo = config_get_uint_value(archivo_FCB, "PUNTERO_DIRECTO");
		liberar_bloque(puntero_directo);
		config_set_value(archivo_FCB, "PUNTERO_DIRECTO", "");
	}
	return;
}

uint32_t conseguir_ptr_secundario_para_indirecto(uint32_t puntero_indirecto, int nro_de_ptr_secundario){
	usleep(atoi(lectura_de_config.RETARDO_ACCESO_BLOQUE) * 1000);
	uint32_t puntero;
	//semaforo bloques
	//pthread_mutex_lock(&mutex_bloques);
	fseek(bloques, puntero_indirecto * super_bloque_info.block_size + sizeof(puntero) * (nro_de_ptr_secundario-1), SEEK_SET);
	fread(&puntero, sizeof(puntero), 1, bloques);
	//pthread_mutex_unlock(&mutex_bloques);
	return puntero;
}

void guardar_ptr_secundario_para_indirecto(uint32_t puntero_secundairo, uint32_t puntero_indirecto, int nro_de_ptr_secundario){
	usleep(atoi(lectura_de_config.RETARDO_ACCESO_BLOQUE) * 1000);
	//semaforo bloques
	//pthread_mutex_lock(&mutex_bloques);
	fseek(bloques, puntero_indirecto * super_bloque_info.block_size + sizeof(puntero_secundairo) * nro_de_ptr_secundario, SEEK_SET);
	fwrite(&puntero_secundairo, sizeof(puntero_secundairo), 1, bloques);				//anoto nuevo puntero_secundario en bloque de puntero_indirecto
	//pthread_mutex_unlock(&mutex_bloques);
}

void agrandas_archivo(t_config* archivo_FCB, char* nombre_archivo, int nuevo_tamanio_archivo){
	int tamanio_archivo = config_get_int_value(archivo_FCB, "TAMANIO_ARCHIVO");

	//CADENA DE IFs, va entrando si cumple condicion
	//entro si no tiene puntero_directo
	if(tamanio_archivo == 0){
		uint32_t puntero_directo = dame_un_bloque_libre();
		config_set_value(archivo_FCB, "PUNTERO_DIRECTO", string_itoa(puntero_directo));
	}
	//entro si tiene puntero_directo, no tiene puntero_indirecto y si el nuevo tamanio requiere de punteros_secundarios
	if(tamanio_archivo <= super_bloque_info.block_size && nuevo_tamanio_archivo > super_bloque_info.block_size){
		uint32_t puntero_indirecto = dame_un_bloque_libre();
		config_set_value(archivo_FCB, "PUNTERO_INDIRECTO", string_itoa(puntero_indirecto));

		int cant_punteros_secundarios = 0;
		int cant_bloques_secundarios_necesarios = (int) ceil((float) nuevo_tamanio_archivo / super_bloque_info.block_size - 1);
		for( ; cant_punteros_secundarios < cant_bloques_secundarios_necesarios ; cant_punteros_secundarios++) {
			uint32_t puntero = dame_un_bloque_libre();
			guardar_ptr_secundario_para_indirecto(puntero, puntero_indirecto, cant_punteros_secundarios);
		}
	}
	//entro si tiene puntero_directo, tiene puntero_indirecto (tiene 1 o + punteros_secundarios)
	if(tamanio_archivo > super_bloque_info.block_size){
		uint32_t puntero_indirecto = config_get_uint_value(archivo_FCB, "PUNTERO_INDIRECTO");

		int cant_punteros_secundarios = (int) ceil((float) tamanio_archivo / super_bloque_info.block_size - 1);
		int cant_bloques_secundarios_necesarios = (int) ceil((float) nuevo_tamanio_archivo / super_bloque_info.block_size - 1);
		for( ; cant_punteros_secundarios < cant_bloques_secundarios_necesarios ; cant_punteros_secundarios++) {
			uint32_t puntero = dame_un_bloque_libre();
			guardar_ptr_secundario_para_indirecto(puntero, puntero_indirecto, cant_punteros_secundarios);
		}
	}
	return;
}

void crear_archivo(char* nombre_archivo) {
	char* path = obtener_path_FCB_sin_free(nombre_archivo);

	//semaforo FCB_default
	//pthread_mutex_lock(&mutex_FCB_default);
	t_config* FCBdefault = iniciar_config("../FCBdefault");
	config_set_value(FCBdefault, "NOMBRE_ARCHIVO", nombre_archivo);
	config_set_value(FCBdefault, "TAMANIO_ARCHIVO", "0");
	config_set_value(FCBdefault, "PUNTERO_DIRECTO", "");
	config_set_value(FCBdefault, "PUNTERO_INDIRECTO", "");
	config_save_in_file(FCBdefault, path);
	config_destroy(FCBdefault);					//cierro el FCB
	free(path);
	//pthread_mutex_unlock(&mutex_FCB_default);
}

bool existe_archivo(char* nombre_archivo) {
	char* path = obtener_path_FCB_sin_free(nombre_archivo);
	bool retorno = archivo_se_puede_leer(path);
	free(path);
	return retorno;
}

char* obtener_path_FCB_sin_free(char* nombre_archivo){
	char* path = malloc(strlen(lectura_de_config.PATH_FCB) + strlen(nombre_archivo));
	strcpy(path, lectura_de_config.PATH_FCB);
	strcat(path, nombre_archivo);		//se asume que en la FS.config el path_fcb tiene / al final quedando: "/home/utnso/fs/fcb/"
	return path;
}

char* leer_archivo(char* nombre_archivo, int apartir_de_donde_leer, int cuanto_leer) {
	char* path = obtener_path_FCB_sin_free(nombre_archivo);
	t_config* archivo_FCB = iniciar_config(path);
	free(path);

	int cantidad_leida = 0; //para ir deplazandome en el buffer cuando ya leí
	//FAIL CHECKERS
	int tamanio_archivo = config_get_int_value(archivo_FCB, "TAMANIO_ARCHIVO");
	if (tamanio_archivo == 0){
		config_destroy(archivo_FCB);
		log_error(logger, "ERROR, tamanio_archivo == 0");
		return "";
	}
	if (apartir_de_donde_leer > tamanio_archivo) {
		log_error(logger, "ERROR, apartir_de_donde_leer es > al tamanio_archivo");
		return "";
	}
	if (apartir_de_donde_leer + cuanto_leer > tamanio_archivo) {
		cuanto_leer = tamanio_archivo - apartir_de_donde_leer;		//deberia tirar error?
		log_warning(logger, "leo hasta fin archivo\n");
	}

	//LECTURA
	const int cuanto_leer_cte = cuanto_leer;
	char* buffer = calloc(1, cuanto_leer_cte);
	//entro si leo de puntero_directo
	if (apartir_de_donde_leer < super_bloque_info.block_size){
		uint32_t puntero_directo = config_get_uint_value(archivo_FCB, "PUNTERO_DIRECTO");
		fseek(bloques, puntero_directo * super_bloque_info.block_size + apartir_de_donde_leer, SEEK_SET);

		leer_bloque(buffer, puntero_directo, apartir_de_donde_leer, &cuanto_leer, &cantidad_leida);
		leer_indirecto(buffer, archivo_FCB, PRIMER_BLOQUE_SECUNDARIO, &cuanto_leer, &cantidad_leida);
	}
	//entro si leo de puntero indirecto y cualquier puntero secundario
	else {
		int bloque_secundario_inicial = (int) ceil((float) apartir_de_donde_leer / super_bloque_info.block_size - 1);
		int apartir_de_donde_leer_bloque_inicial = apartir_de_donde_leer % super_bloque_info.block_size;
		apartir_de_donde_leer_bloque_inicial==0? bloque_secundario_inicial++ : bloque_secundario_inicial;	//pq si apartir es 0, el bloque inicial va a dar el anterior

		uint32_t puntero_indirecto = config_get_uint_value(archivo_FCB, "PUNTERO_INDIRECTO");
		uint32_t puntero_secundario = conseguir_ptr_secundario_para_indirecto(puntero_indirecto, bloque_secundario_inicial);

		leer_bloque(buffer, puntero_secundario, apartir_de_donde_leer_bloque_inicial, &cuanto_leer, &cantidad_leida);
		leer_indirecto(buffer, archivo_FCB, bloque_secundario_inicial+1, &cuanto_leer, &cantidad_leida);
	}
	config_destroy(archivo_FCB);
	return buffer;	//el free se hace en el switch, despues de pasarle el contenido a memoria
}

void leer_bloque(char* buffer, uint32_t puntero, int apartir_de_donde_leer_relativo_a_bloque, int* cuanto_leer, int* cantidad_leida){
	usleep(atoi(lectura_de_config.RETARDO_ACCESO_BLOQUE) * 1000);
	//semaforo bloques
	//pthread_mutex_lock(&mutex_bloques);
	fseek(bloques, puntero * super_bloque_info.block_size + apartir_de_donde_leer_relativo_a_bloque, SEEK_SET);

	char* buffer1;
	if(apartir_de_donde_leer_relativo_a_bloque + *cuanto_leer > super_bloque_info.block_size){
		int cant_disponible_leer_de_puntero = super_bloque_info.block_size - apartir_de_donde_leer_relativo_a_bloque;

		buffer1 = calloc(1, cant_disponible_leer_de_puntero);
		fread(buffer1, cant_disponible_leer_de_puntero, 1, bloques);
		memcpy(buffer + *cantidad_leida, buffer1, cant_disponible_leer_de_puntero);

		*cantidad_leida += cant_disponible_leer_de_puntero;
		*cuanto_leer -= cant_disponible_leer_de_puntero;
	}
	else {
		buffer1 = calloc(1, *cuanto_leer);
		fread(buffer1, *cuanto_leer, 1, bloques);
		memcpy(buffer + *cantidad_leida, buffer1, *cuanto_leer);

		*cantidad_leida += *cuanto_leer;
		*cuanto_leer = 0;
	}
	free(buffer1);
	//pthread_mutex_unlock(&mutex_bloques);
	return;
}

void leer_indirecto(char* buffer, t_config* archivo_FCB, int bloque_secundario_donde_leer, int* cuanto_leer, int* cantidad_leida) {
	uint32_t puntero_indirecto = config_get_uint_value(archivo_FCB, "PUNTERO_INDIRECTO");
	uint32_t puntero_secundario;

	while(*cuanto_leer > 0) {
		puntero_secundario = conseguir_ptr_secundario_para_indirecto(puntero_indirecto, bloque_secundario_donde_leer);
		leer_bloque(buffer, puntero_secundario, DESDE_EL_INICIO, cuanto_leer, cantidad_leida);
		bloque_secundario_donde_leer++;
	}

}

bool archivo_se_puede_leer(char* path)
{
	FILE* f = fopen(path, "r");
	if(f != NULL){
		fclose(f);
		return true;
	} else {
		return false;
	}
}
/*
bool checkear_espacio(int cuanto_escribir) {
	int bloques_necesarios = cuanto_escribir / super_bloque_info.block_size;
	if (cuanto_escribir % super_bloque_info.block_size != 0)
		bloques_necesarios++;
	if (bloques_necesarios > bitarray_get_max_bit(bitarray_de_bitmap))
		return false;
	return true;
}
*/
uint32_t dame_un_bloque_libre() {
	//semaforo bitmap
	//pthread_mutex_lock(&mutex_bitmap);
	for (int i = 0; i < bitarray_get_max_bit(bitarray_de_bitmap); i++) {
		if (bitarray_test_bit(bitarray_de_bitmap, i) == 0) {
			bitarray_set_bit(bitarray_de_bitmap, i);
			msync(bitmap_pointer, tamanioBitmap, MS_SYNC);
			//pthread_mutex_unlock(&mutex_bitmap);
			return i;
		}
	}
	//pthread_mutex_unlock(&mutex_bitmap);
	log_error(logger, "No existe mas espacio en el disco");
	return -1;
}

void liberar_bloque(uint32_t puntero){
	//semaforo bitmap
	//pthread_mutex_lock(&mutex_bitmap);
	bitarray_clean_bit(bitarray_de_bitmap, puntero);
	msync(bitmap_pointer, tamanioBitmap, MS_SYNC);
	//pthread_mutex_unlock(&mutex_bitmap);
}

//debug
int cant_unos_en_bitmap(){
	int contador = 0;
	for (int i = 0; i < bitarray_get_max_bit(bitarray_de_bitmap); i++) {
		if (bitarray_test_bit(bitarray_de_bitmap, i) == 1) {
			contador++;
		}
	}
	return contador;
}



t_instrucciones recibir_cod_op(int socket_cliente)
{
	t_instrucciones cod_op;
	if(recv(socket_cliente, &cod_op, sizeof(t_instrucciones), 0x100) > 0)
		return cod_op;
	else
	{
		close(socket_cliente);
		return -1;
	}
	return -1;
}

void recibir_parametros(t_instrucciones cod_op, char** nombre_archivo, int* tamanio_nuevo_archivo, int* apartir_de_donde_X, int* cuanto_X, int* dir_fisica_memoria){
	size_t size_payload;
	if (recv(kernel, &size_payload, sizeof(size_t), 0) != sizeof(size_t)){
		return;
	}

	void* a_recibir = malloc(size_payload);
	if (recv(kernel, a_recibir, size_payload, 0) != size_payload) {
		free(a_recibir);
		return;
	}

	deserializar_instrucciones_kernel(a_recibir, size_payload, cod_op, nombre_archivo, tamanio_nuevo_archivo, apartir_de_donde_X, cuanto_X, dir_fisica_memoria);

	free(a_recibir);
	return;
}

void deserializar_instrucciones_kernel(void* a_recibir, int size_payload, t_instrucciones cod_op, char** nombre_archivo, int* tamanio_nuevo_archivo, int* apartir_de_donde_X, int* cuanto_X, int* dir_fisica_memoria){
	switch(cod_op){
		case ABRIR:
		case CREAR:
		default:
			int desplazamiento = 0;
			//recibo largo del char* del nombre del file
			size_t largo_nombre;
			memcpy(&largo_nombre, a_recibir + desplazamiento, sizeof(largo_nombre));
			desplazamiento += sizeof(largo_nombre);
			//recibo nombre del file
			char* aux_nombre_file = malloc(largo_nombre);
			memcpy(aux_nombre_file, a_recibir + desplazamiento, largo_nombre);
			desplazamiento += largo_nombre;
			*nombre_archivo = strdup(aux_nombre_file);
			free(aux_nombre_file);

			switch(cod_op){
				case TRUNCAR:
					//recibo nuevo tamanio archivo
					memcpy(tamanio_nuevo_archivo, a_recibir + desplazamiento, sizeof(int));
					desplazamiento += sizeof(int);
					break;
				case LEER:
				case ESCRIBIR:
					//recibo apartir_de_donde_X
					memcpy(apartir_de_donde_X, a_recibir + desplazamiento, sizeof(int));
					desplazamiento += sizeof(int);
					//recibo cuanto_X
					memcpy(cuanto_X, a_recibir + desplazamiento, sizeof(int));
					desplazamiento += sizeof(int);
					//recibo dir_fisica_memoria
					memcpy(dir_fisica_memoria, a_recibir + desplazamiento, sizeof(int));
					desplazamiento += sizeof(int);
					break;
				case ABRIR:
				case CREAR:
				case ERROR:
					break;
			}
			break;
		case ERROR:
			break;
		}
}

void enviar_mensaje_kernel(int kernel, char* msj){
	return;
}
char* leer_de_memoria(int socket_memoria, t_instrucciones cod_op, int cuanto_escribir, int dir_fisica_memoria){
	char* buffer = malloc(cuanto_escribir);
	switch (cuanto_escribir){
	case 1:
		strcpy(buffer, "1");
		break;
	case 2:
		strcpy(buffer, "22");
		break;
	case 3:
		strcpy(buffer, "Que");
		break;
	case 4:
		strcpy(buffer, "bobo");
		break;
	case 5:
		strcpy(buffer, "miras");
		break;
	case 160:
		strcpy(buffer, "Nukess accelerators use 2 powerful magnets and electric fields to propel particles at high speeds, causing them to collide and release energy and radiation, XD.");
		break;
	default:
		strcpy(buffer, "hola");
		break;
	}
	return buffer;
}
void mandar_a_memoria(int socket_memoria, t_instrucciones cod_op, char* buffer, int cuanto_leer, int dir_fisica_memoria){
	return;
}








