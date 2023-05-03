################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/comunicaciones_memoria.c \
../src/conexiones_memoria.c \
../src/configuracion_memoria.c \
../src/memoria.c 

C_DEPS += \
./src/comunicaciones_memoria.d \
./src/conexiones_memoria.d \
./src/configuracion_memoria.d \
./src/memoria.d 

OBJS += \
./src/comunicaciones_memoria.o \
./src/conexiones_memoria.o \
./src/configuracion_memoria.o \
./src/memoria.o 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c src/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/utnso/tp-2023-1c-Los-Matias/shared/src" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src

clean-src:
	-$(RM) ./src/comunicaciones_memoria.d ./src/comunicaciones_memoria.o ./src/conexiones_memoria.d ./src/conexiones_memoria.o ./src/configuracion_memoria.d ./src/configuracion_memoria.o ./src/memoria.d ./src/memoria.o

.PHONY: clean-src

