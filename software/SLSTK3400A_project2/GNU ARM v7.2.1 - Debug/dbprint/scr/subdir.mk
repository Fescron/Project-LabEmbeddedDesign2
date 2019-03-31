################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
/home/brecht/Qsync/Documents/1-Github-Repositories/dbprint/scr/dbprint.c 

OBJS += \
./dbprint/scr/dbprint.o 

C_DEPS += \
./dbprint/scr/dbprint.d 


# Each subdirectory must supply rules for building sources it contributes
dbprint/scr/dbprint.o: /home/brecht/Qsync/Documents/1-Github-Repositories/dbprint/scr/dbprint.c
	@echo 'Building file: $<'
	@echo 'Invoking: GNU ARM C Compiler'
	arm-none-eabi-gcc -g -gdwarf-2 -mcpu=cortex-m0plus -mthumb -std=c99 '-DDEBUG_EFM=1' '-DDEBUG=1' '-DEFM32HG322F64=1' -I"/home/brecht/Programs/SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v2.4//platform/CMSIS/Include" -I"/home/brecht/Programs/SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v2.4//hardware/kit/common/bsp" -I"/home/brecht/Programs/SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v2.4//platform/Device/SiliconLabs/EFM32HG/Include" -I"/home/brecht/Programs/SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v2.4//hardware/kit/SLSTK3400A_EFM32HG/config" -I"/home/brecht/Programs/SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v2.4//platform/emlib/inc" -I/home/brecht/Qsync/Documents/1-Github-Repositories/dbprint/inc -O0 -Wall -c -fmessage-length=0 -mno-sched-prolog -fno-builtin -ffunction-sections -fdata-sections -MMD -MP -MF"dbprint/scr/dbprint.d" -MT"dbprint/scr/dbprint.o" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


