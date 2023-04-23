#include "../include/fileSystem.h"

t_config* superbloque;
//FILE* bitmap;
void* bitmap;
t_bitarray* bitarray_de_bitmap;
FILE* bloques;
//t_config* directorio_FCB;
int tamanioBitmap;

t_log* logger;
t_config* config;
t_fileSystem_config lectura_de_config;

int main(int argc, char** argv) {

	//LECTURA DE CONFIG DEL FILESYSTEM
	
	logger = iniciar_logger("FileSystem.log", "FS");
	config = iniciar_config("../fileSystem.config");

    lectura_de_config = leer_fileSystem_config(config);

	//SE CONECTA AL SERIVDOR MEMORIA

	int socket_memoria = crear_conexion(lectura_de_config.IP_MEMORIA, lectura_de_config.PUERTO_MEMORIA);
    enviar_handshake(socket_memoria, FILESYSTEM);

	//SE HACE SERVIDOR Y ESPERA LA CONEXION DEL KERNEL
	//TODO CREAR UN HILO PARA ESPERAR EL CLIENTE(KERNEL)

	int server = iniciar_servidor("127.0.0.1", lectura_de_config.PUERTO_ESCUCHA);
	puts("Servidor listo para recibir al cliente");
	int kernel = esperar_cliente(server);

	puts("Se conecto el Kernel a FileSystem");

	//CHQUEO DE QUE LOS PATHS A LOS DIFERENTES ARCHIVOS EXISTEN(SUPERBLOQUE, DIRECTORIO_FCB, BITMAP, BLOQUES)

	//SUPERBLOQUE
	if (archivo_se_puede_leer(lectura_de_config.PATH_SUPERBLOQUE)){
		log_info(logger, "SuperBloque existe");
		superbloque = iniciar_config(lectura_de_config.PATH_SUPERBLOQUE);
		super_bloque_info.block_size = config_get_int_value(superbloque, "BLOCK_SIZE");
		super_bloque_info.block_count = config_get_int_value(superbloque, "BLOCK_COUNT");
		log_info(logger, "SuperBloque leido");
	} else {
		log_error(logger, "SuperBloque no esiste");
		//kaboom?
	}

	//BITMAP
	int fd = open(lectura_de_config.PATH_BITMAP, O_CREAT | O_RDWR, 0664);	//abre o crea el archivo en un file descriptor
	if (fd == -1) {
		close(fd);
		log_error(logger, "Error abriendo el bitmap");
		//kaboom?
	}
	double c = (double) super_bloque_info.block_count;
	tamanioBitmap = (int) ceil( c/8.0 ); 	// tener en cuenta si no se necesita en otro lado
	void* bitmap_pointer = mmap(NULL, tamanioBitmap, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	bitarray_de_bitmap = bitarray_create_with_mode((char*) bitmap_pointer, tamanioBitmap, MSB_FIRST);
	close(fd);

	//BLOQUES
	bloques = fopen(lectura_de_config.PATH_BLOQUES, "r+");
	if (bloques){
		log_info(logger, "Bloques existe");

	} else {
		bloques = fopen(lectura_de_config.PATH_BLOQUES, "w+");
	}

	while (1) {
		t_instrucciones cod_op = recibir_operacion(kernel);
		char* nombre_archivo;

		switch (cod_op) {
			case ABRIR:{
				recibir_parametros(kernel);

				if (existe_archivo(nombre_archivo)) {	//existe FCB?
					enviar_mensaje_kernel(kernel, "OK El archivo ya existe");
				} else {
					enviar_mensaje_kernel(kernel, "ERROR El archivo NO existe");
				}
				break;
			}
			case CREAR:{
				recibir_parametros(kernel);
				crear_archivo(nombre_archivo);	//crear FCB y poner tamaño 0 y sin bloques asociados.
				enviar_mensaje_kernel(kernel, "OK Archivo creado");
				break;
			}
			case TRUNCAR:{
				int tamanio_nuevo_archivo;
				recibir_parametros(kernel);
				/*if (tamanio_archivo(nombre_archivo) > tamanio_nuevo_archivo){
					//achicas_archivo();
				}
				else {
					//agrandas_archivo();
				}*/
				enviar_mensaje_kernel(kernel, "OK Archivo truncado");
				break;
			}
			case LEER:{
				int apartir_de_donde_leer;
				int cuanto_leer;
				int dir_fisica_memoria;
				recibir_parametros(kernel);
				char* buffer = leer_archivo(nombre_archivo, apartir_de_donde_leer, cuanto_leer);	//malloc se hace en leer_archivo
				mandar_a_memoria(socket_memoria, ESCRIBIR, buffer, cuanto_leer, dir_fisica_memoria);
				enviar_mensaje_kernel(kernel, "OK Archivo leido");
				free(buffer);
				break;
			}
			case ESCRIBIR:{
				int apartir_de_donde_escribir;
				int cuanto_escribir;
				int dir_fisica_memoria;
				recibir_parametros(kernel);
				char* buffer = leer_de_memoria(socket_memoria, LEER, cuanto_escribir, dir_fisica_memoria);	//malloc se hace en leer_de_memoria
				escribir_archivo(buffer, nombre_archivo, apartir_de_donde_escribir, cuanto_escribir);
				enviar_mensaje_kernel(kernel, "OK Archivo escrito");
				free(buffer);
				break;
			}
			default:
			case ERROR:
				recibir_parametros(kernel);
				break;
		}
		free(nombre_archivo);
		config_destroy(superbloque);
		msync(bitmap_pointer, tamanioBitmap, MS_SYNC);
		munmap(bitmap_pointer, tamanioBitmap);
		fclose(bloques);
		return 0;
	}
	config_destroy(superbloque);
	msync(bitmap_pointer, tamanioBitmap, MS_SYNC);
	munmap(bitmap_pointer, tamanioBitmap);
	fclose(bloques);
	return EXIT_SUCCESS;
}

void crear_archivo(char* nombre_archivo) {
	char* path = obtener_path_FCB_sin_free(nombre_archivo);

	FILE* archivo_a_crear = fopen(path, "w");	//creo el archivo para el FCB
	fclose(archivo_a_crear);

	t_config* fcb_nuevo = iniciar_config(path);	//abro el FCB creado como config y seteo valores de iniciacion
	config_set_value(fcb_nuevo, "NOMBRE_ARCHIVO", nombre_archivo);
	config_set_value(fcb_nuevo, "TAMANIO_ARCHIVO", "0");
	config_set_value(fcb_nuevo, "PUNTERO_DIRECTO", "");
	config_set_value(fcb_nuevo, "PUNTERO_INDIRECTO", "");
	config_destroy(fcb_nuevo);					//cierro el FCB como config
	free(path);
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
	
/*void delete_FCB(char* nombre_archivo) {
	char* path;
	strcpy(path, lectura_de_config.PATH_FCB);
	strcat(path, nombre_archivo);		//se asume que en la FS.config el path_fcb tiene / al final quedando: "/home/utnso/fs/fcb/"
	remove(path);
}*/ //hace falta esto???

char* leer_archivo(char* nombre_archivo, int apartir_de_donde_leer, int cuanto_leer) {
	char* path = obtener_path_FCB_sin_free(nombre_archivo);
	t_config* archivo_FCB = iniciar_config(path);
	free(path);

	int tamanio_archivo = config_get_int_value(archivo_FCB, "TAMANIO_ARCHIVO");
	if (apartir_de_donde_leer > tamanio_archivo) {
		return "ERROR, apartir_de_donde_leer es > al tamanio_archivo";
	}
	if (apartir_de_donde_leer + cuanto_leer > tamanio_archivo) {
		cuanto_leer = tamanio_archivo - apartir_de_donde_leer;		//deberia tirar error?
	}
	uint32_t puntero_directo = config_get_int_value(archivo_FCB, "PUNTERO_DIRECTO");
	uint32_t puntero_indirecto = config_get_int_value(archivo_FCB, "PUNTERO_INDIRECTO");
	config_destroy(archivo_FCB);

	char* buffer = malloc(cuanto_leer);
	if (tamanio_archivo != 0) {
		fseek(bloques, puntero_directo * super_bloque_info.block_size, SEEK_SET);
		if(cuanto_leer < super_bloque_info.block_size){
			fread(buffer, cuanto_leer, 1, bloques);
		}else{
			fread(buffer, super_bloque_info.block_size, 1, bloques);
			leer_indirecto(&buffer, puntero_indirecto, cuanto_leer);
		}
	}
	else {
		//logear error
		exit(EXIT_FAILURE);
	}
	return buffer;	//el free se hace en el switch, despues de pasarle el contenido a memoria
}

void leer_indirecto(char** buffer, uint32_t puntero_indirecto, int cuanto_leer) {

	char * buffer1 = malloc(super_bloque_info.block_size);
	uint32_t dir_bloque;

	t_list* aux = list_create();
	t_list_iterator *lista_bloques = list_iterator_create(aux);	//TODO Fijarse que probablemente este MAL

	int cantidad_de_bloques = (int) ceil((cuanto_leer - super_bloque_info.block_size)/super_bloque_info.block_size);
	
	//agarrar direccion a los bloques de contenido desde el bloque de puntero indirecto
	fseek(bloques, puntero_indirecto * super_bloque_info.block_size, SEEK_SET);
	for(int i = 0; i < cantidad_de_bloques; i++){
		fseek(bloques, puntero_indirecto * super_bloque_info.block_size + i * sizeof(uint32_t), SEEK_SET);
		fread(&dir_bloque, sizeof(uint32_t), 1, bloques);
		list_iterator_add(lista_bloques, &dir_bloque);
	}

	//lees los bloques de contenido, teniendo en cuenta que el ultimo bloque podria
	//no tener q leerse por completo
	dir_bloque = list_iterator_index(0);
	while (true) {
		fseek(bloques, dir_bloque * super_bloque_info.block_size, SEEK_SET);
		if(cuanto_leer >= super_bloque_info.block_size){
			fread(buffer1, super_bloque_info.block_size, 1, bloques);
		} else {
			fread(buffer1, cuanto_leer, 1, bloques);
		}
		strcat(*buffer, buffer1);

		cuanto_leer -= super_bloque_info.block_size;
		if(list_iterator_has_next(lista_bloques))
			dir_bloque = (uint32_t) list_iterator_next(lista_bloques);		//TODO
		else
			break;
	}
	free(buffer1);
}

void escribir_archivo(char* buffer, char* nombre_archivo, int apartir_de_donde_escribir, int cuanto_escribir) {
	char* cuanto_escribir_string, puntero_directo_string, puntero_indirecto_string;
	int cantidad_escrita = 0;


	char* path = obtener_path_FCB_sin_free(nombre_archivo);
	t_config* archivo_FCB = iniciar_config(path);
	free(path);
	
	int tamanio_archivo = config_get_int_value(archivo_FCB, "TAMANIO_ARCHIVO");

	if(checkear_espacio(cuanto_escribir))//chequea que pueda escribi todo!!!!
	{
		if (tamanio_archivo == 0){		//solo para archivos NUEVOS
			cuanto_escribir_string = string_itoa(cuanto_escribir);
			config_set_value(archivo_FCB, "TAMANIO_ARCHIVO", cuanto_escribir_string);

			uint32_t puntero_directo = dame_un_bloque_libre();
			puntero_directo_string = string_itoa(puntero_directo);
			config_set_value(archivo_FCB, "PUNTERO_DIRECTO", puntero_directo_string);
			escribir_bloque(&buffer, puntero_directo, &cuanto_escribir, &cantidad_escrita, super_bloque_info.block_size);
			
			if(cuanto_escribir > 0){
				uint32_t puntero_indirecto = dame_un_bloque_libre();
				puntero_indirecto_string = string_itoa(puntero_indirecto);
				config_set_value(archivo_FCB, "PUNTERO_INDIRECTO", puntero_indirecto_string);

				for(int cant_punteros_secundarios = 0 ; cuanto_escribir > 0 ; cant_punteros_secundarios++) {
					uint32_t puntero = dame_un_bloque_libre();
					fseek(bloques, puntero_indirecto * super_bloque_info.block_size + sizeof(puntero) * cant_punteros_secundarios, SEEK_SET);
					fwrite(puntero, sizeof(puntero), 1, bloques);				//anoto nuevo puntero_secundario en bloque de puntero_indirecto
					escribir_bloque(&buffer, puntero, &cuanto_escribir, &cantidad_escrita, super_bloque_info.block_size);
				}
			}
		}
		else {
			uint32_t puntero_directo = (uint32_t) config_get_int_value(archivo_FCB, "PUNTERO_DIRECTO");
			uint32_t puntero_indirecto = (uint32_t) config_get_int_value(archivo_FCB, "PUNTERO_INDIRECTO");

			cuanto_escribir_string = intToCharAsterisco((unsigned int) (cuanto_escribir + tamanio_archivo));
			config_set_value(archivo_FCB, "TAMANIO_ARCHIVO", cuanto_escribir_string);

			int cantidad_de_bloques_ocupados = (tamanio_archivo / super_bloque_info.block_size);	//ej: 120 / 64 = 1 (trunca)
			if(cantidad_de_bloques_ocupados == 0) {
				escribir_bloque(&buffer, puntero_directo, &cuanto_escribir, &cantidad_escrita, super_bloque_info.block_size);
			}
			else {
				uint32_t puntero;
				cantidad_de_bloques_ocupados--;	//resto el bloque correspondiente al puntero directo
				fseek(bloques, puntero_indirecto * super_bloque_info.block_size + sizeof(puntero) * cantidad_de_bloques_ocupados, SEEK_SET);
				fread(&puntero, sizeof(puntero), 1, bloques);
				
				int espacio_libre_en_bloque = tamanio_archivo - cantidad_de_bloques_ocupados * super_bloque_info.block_size;
				escribir_bloque(&buffer, puntero, &cuanto_escribir, &cantidad_escrita, espacio_libre_en_bloque);
				cantidad_de_bloques_ocupados++;

				for(int cant_punteros_secundarios = cantidad_de_bloques_ocupados ; cuanto_escribir > 0 ; cant_punteros_secundarios++) {
					uint32_t puntero = dame_un_bloque_libre();
					fseek(bloques, puntero_indirecto * super_bloque_info.block_size + sizeof(puntero) * cant_punteros_secundarios, SEEK_SET);
					fwrite(puntero, sizeof(puntero), 1, bloques);				//anoto nuevo puntero_secundario en bloque de puntero_indirecto
					escribir_bloque(&buffer, puntero, &cuanto_escribir, &cantidad_escrita, super_bloque_info.block_size);
				}
			}
		}
	}
	
	free(cuanto_escribir_string);
	free(puntero_directo_string);
	free(puntero_indirecto_string);
	config_destroy(archivo_FCB);
	
	/*while (cuanto_escribir != 0) {
		fseek(bloques, puntero_directo * super_bloque_info.block_size, SEEK_SET);
		if(cuanto_escribir < super_bloque_info.block_size){
			fwrite(buffer, cuanto_escribir, 1, bloques);
		}else{
			fwrite(buffer, super_bloque_info.block_size, 1, bloques);
			escribir_indirecto(&buffer, puntero_indirecto, cuanto_escribir);
		}
	}
	else {
		//logear error
		exit(EXIT_FAILURE);
	}*/
}

bool archivo_se_puede_leer(char* path)
{
	return (!access(path, R_OK ));
}

bool archivo_se_puede_escribir(char* path)
{
	return (!access(path, W_OK ));
}

//Prueba int to char*
// seguramente haya q pasar por referencia el char*
char* intToCharAsterisco(unsigned int numero) {

	numero = abs(numero);

	int digitos = cantidad_de_digitos(numero);

	char* string = malloc(digitos + 1);
	int digito;

	if(numero == 0)
		return "0";

	for (int i = 0, j = digitos - 1, copiaNumero = numero; numero > 0; i++, j--) {
        copiaNumero /= 10;
        digito = numero - (copiaNumero * 10);
        string[j] = digito  + '0';
        numero = copiaNumero;
    }

    return string;
}

int cantidad_de_digitos(unsigned int numero) { 
  int cociente = 0, divisor = 1, contador = 0;

  while (cociente !=1){
    contador++;
    divisor = divisor * 10;
    cociente = numero / divisor;

    if (cociente == 1){
    	return contador + 1;
    }

    if (cociente == 0){
    	return contador;
    }

    if (numero < 10){
    	return 1;
    }
  }
  return 0;
}

bool checkear_espacio(int cuanto_escribir) {
	int bloques_necesarios = cuanto_escribir / super_bloque_info.block_size;
	if (cuanto_escribir % super_bloque_info.block_size != 0)
		bloques_necesarios++;
	if (bloques_necesarios > bitarray_get_max_bit(bitarray_de_bitmap))
		return false;
	return true;
}

uint32_t dame_un_bloque_libre() {
	for (int i = 0; i < bitarray_get_max_bit(bitarray_de_bitmap); i++) {
		if (bitarray_test_bit(bitarray_de_bitmap, i) == 0) {
			bitarray_set_bit(bitarray_de_bitmap, i);
			return i;
		}
	}
	return -1;	
}

void escribir_bloque(char* buffer, uint32_t puntero, int* cuanto_escribir, int* cantidad_escrita, int espacio_libre_en_bloque) {
	int a_partir_de_donde_escribir = super_bloque_info.block_size - espacio_libre_en_bloque;
	fseek(bloques, puntero * super_bloque_info.block_size + a_partir_de_donde_escribir, SEEK_SET);

	char* copia_buffer = (char*) string_substring(buffer, (int)*cantidad_escrita, (int)*cuanto_escribir);

	if(*cuanto_escribir < espacio_libre_en_bloque){
		fwrite(copia_buffer, *cuanto_escribir, 1, bloques);
		*cantidad_escrita += *cuanto_escribir;
		*cuanto_escribir = 0;
	}
	else {
		fwrite(copia_buffer, espacio_libre_en_bloque, 1, bloques);
		*cantidad_escrita += espacio_libre_en_bloque;
		*cuanto_escribir -= espacio_libre_en_bloque;
	}
	free(copia_buffer);
}

int espacio_libre_en_bloque(uint32_t puntero) {
	int espacio_libre = 0;
	fseek(bloques, puntero * super_bloque_info.block_size, SEEK_SET);
	char* buffer = malloc(super_bloque_info.block_size);
	fread(buffer, super_bloque_info.block_size, 1, bloques);
	for (int i = 0; i < super_bloque_info.block_size; i++) {
		if (buffer[i] == '\0')
			espacio_libre++;
	}
	free(buffer);
	return espacio_libre;
}


void recibir_parametros(int socket_kernel){
	return;
}
void enviar_mensaje_kernel(int socket_kernel, char* msj){
	return;
}
char* leer_de_memoria(int socket_memoria, t_instrucciones LEER, int cuanto_escribir, int dir_fisica_memoria){
	return "hey";
}
void mandar_a_memoria(int socket_memoria, t_instrucciones ESCRIBIR, char* buffer, int cuanto_leer, int dir_fisica_memoria){
	return;
}







