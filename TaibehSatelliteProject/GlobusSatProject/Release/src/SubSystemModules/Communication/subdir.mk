################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/SubSystemModules/Communication/AckHandler.c \
../src/SubSystemModules/Communication/ActUponCommand.c \
../src/SubSystemModules/Communication/Beacon.c \
../src/SubSystemModules/Communication/CommandDictionary.c \
../src/SubSystemModules/Communication/SatCommandHandler.c \
../src/SubSystemModules/Communication/SatDataTx.c \
../src/SubSystemModules/Communication/TRXVU.c 

OBJS += \
./src/SubSystemModules/Communication/AckHandler.o \
./src/SubSystemModules/Communication/ActUponCommand.o \
./src/SubSystemModules/Communication/Beacon.o \
./src/SubSystemModules/Communication/CommandDictionary.o \
./src/SubSystemModules/Communication/SatCommandHandler.o \
./src/SubSystemModules/Communication/SatDataTx.o \
./src/SubSystemModules/Communication/TRXVU.o 

C_DEPS += \
./src/SubSystemModules/Communication/AckHandler.d \
./src/SubSystemModules/Communication/ActUponCommand.d \
./src/SubSystemModules/Communication/Beacon.d \
./src/SubSystemModules/Communication/CommandDictionary.d \
./src/SubSystemModules/Communication/SatCommandHandler.d \
./src/SubSystemModules/Communication/SatDataTx.d \
./src/SubSystemModules/Communication/TRXVU.d 


# Each subdirectory must supply rules for building sources it contributes
src/SubSystemModules/Communication/%.o: ../src/SubSystemModules/Communication/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=arm926ej-s -Os -fmessage-length=0 -ffunction-sections -Wall -Wextra -DTRACE_LEVEL=5 -Dat91sam9g20 -Dsdram -D'BASE_REVISION_NUMBER=$(REV)' -D'BASE_REVISION_HASH_SHORT=$(REVHASH_SHORT)' -D'BASE_REVISION_HASH=$(REVHASH)' -I"C:\ISIS\workspace\Taibeh\25.3.2020\GlobusSatProject\src" -I"C:/ISIS/workspace/Taibeh/25.3.2020/GlobusSatProject/..//hal/freertos/include" -I"C:/ISIS/workspace/Taibeh/25.3.2020/GlobusSatProject/..//hal/hal/include" -I"C:/ISIS/workspace/Taibeh/25.3.2020/GlobusSatProject/..//hal/at91/include" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


