################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/abrir_crear_archivo.c \
../src/conexiones_fileSystem.c \
../src/configuracion_fileSystem.c \
../src/escribir_archivo.c \
../src/fileSystem.c \
../src/leer_archivo.c \
../src/manejar_mensajes.c \
../src/manejo_punteros.c \
../src/truncar_archivo.c 

C_DEPS += \
./src/abrir_crear_archivo.d \
./src/conexiones_fileSystem.d \
./src/configuracion_fileSystem.d \
./src/escribir_archivo.d \
./src/fileSystem.d \
./src/leer_archivo.d \
./src/manejar_mensajes.d \
./src/manejo_punteros.d \
./src/truncar_archivo.d 

OBJS += \
./src/abrir_crear_archivo.o \
./src/conexiones_fileSystem.o \
./src/configuracion_fileSystem.o \
./src/escribir_archivo.o \
./src/fileSystem.o \
./src/leer_archivo.o \
./src/manejar_mensajes.o \
./src/manejo_punteros.o \
./src/truncar_archivo.o 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c src/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/utnso/tp-2023-1c-Los-Matias/shared/src" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src

clean-src:
	-$(RM) ./src/abrir_crear_archivo.d ./src/abrir_crear_archivo.o ./src/conexiones_fileSystem.d ./src/conexiones_fileSystem.o ./src/configuracion_fileSystem.d ./src/configuracion_fileSystem.o ./src/escribir_archivo.d ./src/escribir_archivo.o ./src/fileSystem.d ./src/fileSystem.o ./src/leer_archivo.d ./src/leer_archivo.o ./src/manejar_mensajes.d ./src/manejar_mensajes.o ./src/manejo_punteros.d ./src/manejo_punteros.o ./src/truncar_archivo.d ./src/truncar_archivo.o

.PHONY: clean-src

