#include "../include/pcb_kernel.h"

t_pcb* generar_pcb(t_list* proceso, int config_estimado_de_proxima_rafaga) {
	t_pcb* pcb; //= malloc() ?

	//pcb->pid = generar_pid();
	pcb->instrucciones = proceso;
	pcb->pc = 0;
	pcb->registros_cpu = generar_registros_cpu();
	pcb->tabla_de_segmentos = list_create(); // VER, porque lo debe crear la Memoria
	pcb->estimado_de_proxima_rafaga = config_estimado_de_proxima_rafaga; // Se recalculará a medida que ocurra el proceso
	pcb->tiempo_de_llegada_a_ready = 0;
	pcb->archivos_abiertos = list_create();

	return pcb;
}

int generar_pid() { // Hacer con archivos para que sea único e irrepetible
	FILE* f_pid_unico = fopen("../pid_unico.txt", "r+");

	char* string_ultimo_pid_leido; // = malloc() ?
	fscanf(f_pid_unico, "%s", string_ultimo_pid_leido);
	int int_ultimo_pid_leido = atoi(string_ultimo_pid_leido);

	int nuevo_pid = int_ultimo_pid_leido + 1;
	fseek(f_pid_unico, 0, SEEK_SET);
	fprintf(f_pid_unico, "%s", string_itoa(nuevo_pid));

	fclose(f_pid_unico);

	return nuevo_pid;
}

t_registros_cpu* generar_registros_cpu() {
	t_registros_cpu* registros_a_generar; // = malloc() ?

	registros_a_generar->AX  = (char*)malloc(4 * sizeof(char));
	//char* unaVariableParaProbar = (char*)malloc(4 * sizeof(char));
	registros_a_generar->BX  = (char*)malloc(4 * sizeof(char));
	registros_a_generar->CX  = (char*)malloc(4 * sizeof(char));
	registros_a_generar->DX  = (char*)malloc(4 * sizeof(char));

	registros_a_generar->EAX = (char*)malloc(8 * sizeof(char));
	registros_a_generar->EBX = (char*)malloc(8 * sizeof(char));
	registros_a_generar->ECX = (char*)malloc(8 * sizeof(char));
	registros_a_generar->EDX = (char*)malloc(8 * sizeof(char));

	registros_a_generar->RAX = (char*)malloc(16 * sizeof(char));
	registros_a_generar->RBX = (char*)malloc(16 * sizeof(char));
	registros_a_generar->RCX = (char*)malloc(16 * sizeof(char));
	registros_a_generar->RDX = (char*)malloc(16 * sizeof(char));

	return registros_a_generar;
}
