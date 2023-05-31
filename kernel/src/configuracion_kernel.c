#include "../include/configuracion_kernel.h"
#include <commons/log.h>
#include <commons/collections/queue.h>

//SEMAFOROS////////////////////////////////
pthread_mutex_t mutex_contador_pid; //para que dos procesos no tengan el mismo ID
pthread_mutex_t mutex_ready_list; //por si quieren agregar y eliminar de la lista al mismo tiempo.
pthread_mutex_t mutex_new_queue;
pthread_mutex_t mutex_cantidad_de_reads_writes;
pthread_mutex_t mutex_msj_memoria;
pthread_mutex_t mutex_pcbs_en_io;
sem_t sem_cant_ready;
sem_t sem_cant_new;
sem_t sem_multiprogramacion;
sem_t sem_respuesta_fs;
sem_t sem_compactacion;
///////////////////////////////////////////

//CONFIG///////////////////////////////////
t_kernel_config lectura_de_config;
///////////////////////////////////////////

//LOGGER///////////////////////////////////
t_log* logger;

//SOCKETS//////////////////////////////////
int socket_memoria;
int socket_cpu;
int socket_fileSystem;
///////////////////////////////////////////

t_queue* new_queue;
t_list* ready_list;

t_list* list_recursos;
t_list* list_archivos;

t_list* pcbs_en_io;
///////////////////////////////////////////
t_pcb* proximo_pcb_a_ejecutar_forzado = NULL;
t_msj_kernel_fileSystem respuesta_fs_global;
int cantidad_de_reads_writes = 0;

///////////////////////////////////////////
t_kernel_config leer_kernel_config(t_config* config) {
    t_kernel_config lectura_de_config;

    lectura_de_config.IP_MEMORIA        = strdup(config_get_string_value(config, "IP_MEMORIA"));
    lectura_de_config.PUERTO_MEMORIA    = strdup(config_get_string_value(config, "PUERTO_MEMORIA"));
    lectura_de_config.IP_CPU            = strdup(config_get_string_value(config, "IP_CPU"));
    lectura_de_config.PUERTO_CPU        = strdup(config_get_string_value(config, "PUERTO_CPU"));
    lectura_de_config.IP_FILESYSTEM     = strdup(config_get_string_value(config, "IP_FILESYSTEM"));
    lectura_de_config.PUERTO_FILESYSTEM = strdup(config_get_string_value(config, "PUERTO_FILESYSTEM"));

    lectura_de_config.PUERTO_ESCUCHA              = strdup(config_get_string_value(config, "PUERTO_ESCUCHA"));
    lectura_de_config.ALGORITMO_PLANIFICACION     = strdup(config_get_string_value(config, "ALGORITMO_PLANIFICACION"));
    lectura_de_config.ESTIMACION_INICIAL          = config_get_double_value(config, "ESTIMACION_INICIAL");
    lectura_de_config.HRRN_ALFA                   = config_get_double_value(config, "HRRN_ALFA");
    lectura_de_config.GRADO_MAX_MULTIPROGRAMACION = config_get_int_value(config, "GRADO_MAX_MULTIPROGRAMACION");

    char* recursos             = strdup(config_get_string_value(config, "RECURSOS")); //"[elem1, elem2, ...]"
    char* recursos_sin_espacio = string_replace(recursos, " " , ""); //"[elem1,elem2,...]"
    free(recursos);
    lectura_de_config.RECURSOS = string_get_string_as_array(recursos_sin_espacio); //["elem1", "elem", ...]
    free(recursos_sin_espacio); //TODO: REVISAR ESTE FREE (valgrind no se queja)

    char* instancias_recursos             = strdup(config_get_string_value(config, "INSTANCIAS_RECURSOS"));
	char* instancias_recursos_sin_espacio = string_replace(instancias_recursos, " " , "");
	free(instancias_recursos);
	lectura_de_config.INSTANCIAS_RECURSOS = string_get_string_as_array(instancias_recursos_sin_espacio);
	free(instancias_recursos_sin_espacio); //TODO: REVISAR ESTE FREE (valgrind no se queja)

    return lectura_de_config;
}

void init_semaforos() {
	pthread_mutex_init(&mutex_contador_pid, NULL);
	pthread_mutex_init(&mutex_ready_list, NULL);
	pthread_mutex_init(&mutex_new_queue, NULL);
	pthread_mutex_init(&mutex_cantidad_de_reads_writes, NULL);
	pthread_mutex_init(&mutex_msj_memoria, NULL);
	pthread_mutex_init(&mutex_pcbs_en_io, NULL);

	sem_init(&sem_cant_ready, 0, 0);
	sem_init(&sem_cant_new, 0, 0);
	sem_init(&sem_multiprogramacion, 0, lectura_de_config.GRADO_MAX_MULTIPROGRAMACION);
	sem_init(&sem_respuesta_fs, 0, 0);
	sem_init(&sem_compactacion, 0, 1);
}

void init_estados() {
	new_queue = queue_create();
	ready_list = list_create();

	list_recursos = list_create();
	list_archivos = list_create();

	pcbs_en_io = list_create();

	t_recurso* recurso;
	for(int i = 0; i < string_array_size(lectura_de_config.RECURSOS); i++) {
		recurso = malloc(sizeof(t_recurso));

		recurso->nombre = lectura_de_config.RECURSOS[i]; //Hay que hacer strdup?
		recurso->cantidad_disponibles = atoi(lectura_de_config.INSTANCIAS_RECURSOS[i]);
		recurso->cola_bloqueados = queue_create();

		list_add(list_recursos, recurso); //no hace falta mutex
	}
}

void liberar_estructura_config(t_kernel_config config){
	free(config.IP_MEMORIA);
	free(config.PUERTO_MEMORIA);
	free(config.IP_CPU);
	free(config.PUERTO_CPU);
	free(config.IP_FILESYSTEM);
	free(config.PUERTO_FILESYSTEM);
	free(config.PUERTO_ESCUCHA);
	free(config.ALGORITMO_PLANIFICACION);
	//free(config.RECURSOS);
	string_array_destroy(config.RECURSOS); //TODO
	//free(config.INSTANCIAS_RECURSOS); //hay que liberar todos
	string_array_destroy(config.INSTANCIAS_RECURSOS);
}

void log_pids() {
	char* pids = obtener_pids(ready_list);
	log_info(logger, "Cola Ready %s: [%s]", lectura_de_config.ALGORITMO_PLANIFICACION, pids); //log obligatorio
	free(pids);
}

char* obtener_pids(t_list* pcbs) {
    t_pcb* pcb;
	char* pids = string_new();

	for(int i = 0; i < list_size(pcbs); i++) {
		pcb = list_get(pcbs, i);
		char* pid = string_itoa(pcb->pid);
		string_append(&pids, pid);
		free(pid);
		if(i != list_size(pcbs) - 1) { //Para que no ponga coma a lo ultimo
			string_append(&pids, ", ");
		}
	}

	return pids;
}

void mantener_pcb_en_exec(t_pcb* pcb_recibido) {
	proximo_pcb_a_ejecutar_forzado = pcb_recibido;
	sem_post(&sem_cant_ready); //Al no pasar por la funcion ready_list_push hay que hacerlo manualmente, vuelve a ejecutar sin pasar por ready
}

void ready_list_push(t_pcb* pcb_recibido) {
	char* estado_anterior;
	if(pcb_recibido->pc == 0) { //si el proceso viene de New
		estado_anterior = strdup("NEW");
	}
	else { //Si no es la primera vez
		estado_anterior = strdup("BLOCK");
		//calcular_prox_rafaga(pcb_recibido); //S se calcula solo al que entra a Ready. Si es la primera vez que entra a Ready, se mantiene la Estimación Inicial
	}

	log_info(logger, "PID: %d - Estado Anterior: %s - Estado Actual: READY", pcb_recibido->pid, estado_anterior); //log obligatorio
	free(estado_anterior);

	pcb_recibido->tiempo_llegada_ready = time(NULL); //Actualizo el momento en que llega a Ready

	calcular_prox_rafaga(pcb_recibido); //S se calcula solo al que entra a Ready. Si es la primera vez que entra a Ready, se mantiene la Estimación Inicial

	list_push_con_mutex(ready_list, pcb_recibido, &mutex_ready_list);
	log_pids(); //log obligatorio

	sem_post(&sem_cant_ready); //Dice que hay un nuevo proceso en Ready
}

void calcular_prox_rafaga(t_pcb* pcb) {
	log_error(logger, "TIEMPO REAL DE EJECUCION: %f PID: %d", pcb->tiempo_real_ejecucion, pcb->pid);
	double alpha = lectura_de_config.HRRN_ALFA;
	pcb->estimado_prox_rafaga = alpha * pcb->tiempo_real_ejecucion + (1-alpha) * pcb->estimado_prox_rafaga; //pcb->estimado_prox_rafaga aca es como el estimado anterior
	log_error(logger, "ESTIMADO PROX RAFAGA: %f PID: %d", pcb->estimado_prox_rafaga,pcb->pid);
}

