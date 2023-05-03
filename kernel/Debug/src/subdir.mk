################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/comunicaciones_kernel.c \
../src/conexiones_kernel.c \
../src/configuracion_kernel.c \
../src/kernel.c \
../src/planificacion_corto.c \
../src/planificacion_largo.c 

C_DEPS += \
./src/comunicaciones_kernel.d \
./src/conexiones_kernel.d \
./src/configuracion_kernel.d \
./src/kernel.d \
./src/planificacion_corto.d \
./src/planificacion_largo.d 

OBJS += \
./src/comunicaciones_kernel.o \
./src/conexiones_kernel.o \
./src/configuracion_kernel.o \
./src/kernel.o \
./src/planificacion_corto.o \
./src/planificacion_largo.o 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c src/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/utnso/tp-2023-1c-Los-Matias/shared/src" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src

clean-src:
	-$(RM) ./src/comunicaciones_kernel.d ./src/comunicaciones_kernel.o ./src/conexiones_kernel.d ./src/conexiones_kernel.o ./src/configuracion_kernel.d ./src/configuracion_kernel.o ./src/kernel.d ./src/kernel.o ./src/planificacion_corto.d ./src/planificacion_corto.o ./src/planificacion_largo.d ./src/planificacion_largo.o

.PHONY: clean-src

