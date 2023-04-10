################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/configuracion_consola.c \
../src/consola.c 

C_DEPS += \
./src/configuracion_consola.d \
./src/consola.d 

OBJS += \
./src/configuracion_consola.o \
./src/consola.o 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c src/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/utnso/Documents/tp-2023-1c-Los-Matias/shared/src" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src

clean-src:
	-$(RM) ./src/configuracion_consola.d ./src/configuracion_consola.o ./src/consola.d ./src/consola.o

.PHONY: clean-src

