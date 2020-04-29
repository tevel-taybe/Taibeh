################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/main.c 

OBJS += \
./src/main.o 

C_DEPS += \
./src/main.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=arm926ej-s -Os -fmessage-length=0 -ffunction-sections -Wall -Wextra -DTRACE_LEVEL=5 -Dat91sam9g20 -Dsdram -D'BASE_REVISION_NUMBER=$(REV)' -D'BASE_REVISION_HASH_SHORT=$(REVHASH_SHORT)' -D'BASE_REVISION_HASH=$(REVHASH)' -I"C:\ISIS\workspace\Taibeh\25.3.2020\satellite-subsystems\demo\src" -I"C:/ISIS/workspace/Taibeh/25.3.2020/satellite-subsystems/demo/../..//hal/at91/include" -I"C:/ISIS/workspace/Taibeh/25.3.2020/satellite-subsystems/demo/../..//hal/hal/include" -I"C:/ISIS/workspace/Taibeh/25.3.2020/satellite-subsystems/demo/../..//hal/freertos/include" -I"C:/ISIS/workspace/Taibeh/25.3.2020/satellite-subsystems/demo/../..//hal/hcc/include" -I"C:/ISIS/workspace/Taibeh/25.3.2020/satellite-subsystems/demo/..//satellite-subsystems/include" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


