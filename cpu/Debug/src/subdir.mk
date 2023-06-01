################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/configuracion_cpu.c \
../src/cpu.c \
../src/ejecucion_instrucciones.c \
../src/escuchador_de_memoria.c 

C_DEPS += \
./src/configuracion_cpu.d \
./src/cpu.d \
./src/ejecucion_instrucciones.d \
./src/escuchador_de_memoria.d 

OBJS += \
./src/configuracion_cpu.o \
./src/cpu.o \
./src/ejecucion_instrucciones.o \
./src/escuchador_de_memoria.o 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c src/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/utnso/tp-2023-1c-Los-Matias/shared/src" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src

clean-src:
	-$(RM) ./src/configuracion_cpu.d ./src/configuracion_cpu.o ./src/cpu.d ./src/cpu.o ./src/ejecucion_instrucciones.d ./src/ejecucion_instrucciones.o ./src/escuchador_de_memoria.d ./src/escuchador_de_memoria.o

.PHONY: clean-src

