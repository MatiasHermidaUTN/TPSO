################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/comunicaciones_consola.c \
../src/configuracion_consola.c \
../src/consola.c \
../src/parser.c 

C_DEPS += \
./src/comunicaciones_consola.d \
./src/configuracion_consola.d \
./src/consola.d \
./src/parser.d 

OBJS += \
./src/comunicaciones_consola.o \
./src/configuracion_consola.o \
./src/consola.o \
./src/parser.o 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c src/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/utnso/Documents/tp-2023-1c-Los-Matias/shared/src" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src

clean-src:
	-$(RM) ./src/comunicaciones_consola.d ./src/comunicaciones_consola.o ./src/configuracion_consola.d ./src/configuracion_consola.o ./src/consola.d ./src/consola.o ./src/parser.d ./src/parser.o

.PHONY: clean-src

