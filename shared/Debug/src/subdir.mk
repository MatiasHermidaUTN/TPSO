################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/diccionario_instrucciones.c \
../src/utils.c 

C_DEPS += \
./src/diccionario_instrucciones.d \
./src/utils.d 

OBJS += \
./src/diccionario_instrucciones.o \
./src/utils.o 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c src/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src

clean-src:
	-$(RM) ./src/diccionario_instrucciones.d ./src/diccionario_instrucciones.o ./src/utils.d ./src/utils.o

.PHONY: clean-src

