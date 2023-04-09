#ifndef CONSOLA_H_
#define CONSOLA_H_

struct consola_config {
    char* IP_KERNEL;
    char* PUERTO_KERNEL;
} t_consola_config;

t_consola_config leer_consola_config(t_config* config);


#endif /* CONSOLA_H_ */
