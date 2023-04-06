################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/kernel/kernel.c 

C_DEPS += \
./src/kernel/kernel.d 

OBJS += \
./src/kernel/kernel.o 


# Each subdirectory must supply rules for building sources it contributes
src/kernel/%.o: ../src/kernel/%.c src/kernel/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-kernel

clean-src-2f-kernel:
	-$(RM) ./src/kernel/kernel.d ./src/kernel/kernel.o

.PHONY: clean-src-2f-kernel

