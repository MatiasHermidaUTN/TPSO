################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/configuracion_kernel.c \
../src/kernel.c 

C_DEPS += \
./src/configuracion_kernel.d \
./src/kernel.d 

OBJS += \
./src/configuracion_kernel.o \
./src/kernel.o 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c src/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/utnso/Documents/tp-2023-1c-Los-Matias/shared/src" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src

clean-src:
	-$(RM) ./src/configuracion_kernel.d ./src/configuracion_kernel.o ./src/kernel.d ./src/kernel.o

.PHONY: clean-src

