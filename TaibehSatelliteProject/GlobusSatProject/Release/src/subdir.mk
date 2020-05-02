################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/InitSystem.c \
../src/TLM_management.c \
../src/main.c 

OBJS += \
./src/InitSystem.o \
./src/TLM_management.o \
./src/main.o 

C_DEPS += \
./src/InitSystem.d \
./src/TLM_management.d \
./src/main.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=arm926ej-s -Os -fmessage-length=0 -ffunction-sections -Wall -Wextra -DTRACE_LEVEL=5 -Dat91sam9g20 -Dsdram -D'BASE_REVISION_NUMBER=$(REV)' -D'BASE_REVISION_HASH_SHORT=$(REVHASH_SHORT)' -D'BASE_REVISION_HASH=$(REVHASH)' -I"C:\ISIS\workspace\Taibeh\25.3.2020\GlobusSatProject\src" -I"C:/ISIS/workspace/Taibeh/25.3.2020/GlobusSatProject/..//hal/freertos/include" -I"C:/ISIS/workspace/Taibeh/25.3.2020/GlobusSatProject/..//hal/hal/include" -I"C:/ISIS/workspace/Taibeh/25.3.2020/GlobusSatProject/..//hal/at91/include" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


