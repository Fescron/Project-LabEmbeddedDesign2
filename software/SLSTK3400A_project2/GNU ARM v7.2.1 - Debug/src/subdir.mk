################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/ADXL362.c \
../src/DS18B20.c \
../src/driftsensor.c \
../src/handlers.c \
../src/main-driftsensor.c \
../src/main.c \
../src/udelay.c \
../src/util.c 

OBJS += \
./src/ADXL362.o \
./src/DS18B20.o \
./src/driftsensor.o \
./src/handlers.o \
./src/main-driftsensor.o \
./src/main.o \
./src/udelay.o \
./src/util.o 

C_DEPS += \
./src/ADXL362.d \
./src/DS18B20.d \
./src/driftsensor.d \
./src/handlers.d \
./src/main-driftsensor.d \
./src/main.d \
./src/udelay.d \
./src/util.d 


# Each subdirectory must supply rules for building sources it contributes
src/ADXL362.o: ../src/ADXL362.c
	@echo 'Building file: $<'
	@echo 'Invoking: GNU ARM C Compiler'
	arm-none-eabi-gcc -g -gdwarf-2 -mcpu=cortex-m0plus -mthumb -std=c99 '-DDEBUG_EFM=1' '-DDEBUG=1' '-DEFM32HG322F64=1' -I"/home/brecht/Programs/SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v2.4//platform/CMSIS/Include" -I"/home/brecht/Programs/SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v2.4//hardware/kit/common/bsp" -I"/home/brecht/Programs/SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v2.4//platform/Device/SiliconLabs/EFM32HG/Include" -I"/home/brecht/Programs/SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v2.4//hardware/kit/SLSTK3400A_EFM32HG/config" -I"/home/brecht/Programs/SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v2.4//platform/emlib/inc" -I/home/brecht/Qsync/Documents/1-Github-Repositories/dbprint/dbprint-inc -O0 -Wall -c -fmessage-length=0 -mno-sched-prolog -fno-builtin -ffunction-sections -fdata-sections -MMD -MP -MF"src/ADXL362.d" -MT"src/ADXL362.o" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/DS18B20.o: ../src/DS18B20.c
	@echo 'Building file: $<'
	@echo 'Invoking: GNU ARM C Compiler'
	arm-none-eabi-gcc -g -gdwarf-2 -mcpu=cortex-m0plus -mthumb -std=c99 '-DDEBUG_EFM=1' '-DDEBUG=1' '-DEFM32HG322F64=1' -I"/home/brecht/Programs/SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v2.4//platform/CMSIS/Include" -I"/home/brecht/Programs/SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v2.4//hardware/kit/common/bsp" -I"/home/brecht/Programs/SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v2.4//platform/Device/SiliconLabs/EFM32HG/Include" -I"/home/brecht/Programs/SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v2.4//hardware/kit/SLSTK3400A_EFM32HG/config" -I"/home/brecht/Programs/SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v2.4//platform/emlib/inc" -I/home/brecht/Qsync/Documents/1-Github-Repositories/dbprint/dbprint-inc -O0 -Wall -c -fmessage-length=0 -mno-sched-prolog -fno-builtin -ffunction-sections -fdata-sections -MMD -MP -MF"src/DS18B20.d" -MT"src/DS18B20.o" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/driftsensor.o: ../src/driftsensor.c
	@echo 'Building file: $<'
	@echo 'Invoking: GNU ARM C Compiler'
	arm-none-eabi-gcc -g -gdwarf-2 -mcpu=cortex-m0plus -mthumb -std=c99 '-DDEBUG_EFM=1' '-DDEBUG=1' '-DEFM32HG322F64=1' -I"/home/brecht/Programs/SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v2.4//platform/CMSIS/Include" -I"/home/brecht/Programs/SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v2.4//hardware/kit/common/bsp" -I"/home/brecht/Programs/SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v2.4//platform/Device/SiliconLabs/EFM32HG/Include" -I"/home/brecht/Programs/SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v2.4//hardware/kit/SLSTK3400A_EFM32HG/config" -I"/home/brecht/Programs/SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v2.4//platform/emlib/inc" -I/home/brecht/Qsync/Documents/1-Github-Repositories/dbprint/dbprint-inc -O0 -Wall -c -fmessage-length=0 -mno-sched-prolog -fno-builtin -ffunction-sections -fdata-sections -MMD -MP -MF"src/driftsensor.d" -MT"src/driftsensor.o" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/handlers.o: ../src/handlers.c
	@echo 'Building file: $<'
	@echo 'Invoking: GNU ARM C Compiler'
	arm-none-eabi-gcc -g -gdwarf-2 -mcpu=cortex-m0plus -mthumb -std=c99 '-DDEBUG_EFM=1' '-DDEBUG=1' '-DEFM32HG322F64=1' -I"/home/brecht/Programs/SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v2.4//platform/CMSIS/Include" -I"/home/brecht/Programs/SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v2.4//hardware/kit/common/bsp" -I"/home/brecht/Programs/SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v2.4//platform/Device/SiliconLabs/EFM32HG/Include" -I"/home/brecht/Programs/SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v2.4//hardware/kit/SLSTK3400A_EFM32HG/config" -I"/home/brecht/Programs/SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v2.4//platform/emlib/inc" -I/home/brecht/Qsync/Documents/1-Github-Repositories/dbprint/dbprint-inc -O0 -Wall -c -fmessage-length=0 -mno-sched-prolog -fno-builtin -ffunction-sections -fdata-sections -MMD -MP -MF"src/handlers.d" -MT"src/handlers.o" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/main-driftsensor.o: ../src/main-driftsensor.c
	@echo 'Building file: $<'
	@echo 'Invoking: GNU ARM C Compiler'
	arm-none-eabi-gcc -g -gdwarf-2 -mcpu=cortex-m0plus -mthumb -std=c99 '-DDEBUG_EFM=1' '-DDEBUG=1' '-DEFM32HG322F64=1' -I"/home/brecht/Programs/SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v2.4//platform/CMSIS/Include" -I"/home/brecht/Programs/SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v2.4//hardware/kit/common/bsp" -I"/home/brecht/Programs/SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v2.4//platform/Device/SiliconLabs/EFM32HG/Include" -I"/home/brecht/Programs/SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v2.4//hardware/kit/SLSTK3400A_EFM32HG/config" -I"/home/brecht/Programs/SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v2.4//platform/emlib/inc" -I/home/brecht/Qsync/Documents/1-Github-Repositories/dbprint/dbprint-inc -O0 -Wall -c -fmessage-length=0 -mno-sched-prolog -fno-builtin -ffunction-sections -fdata-sections -MMD -MP -MF"src/main-driftsensor.d" -MT"src/main-driftsensor.o" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/main.o: ../src/main.c
	@echo 'Building file: $<'
	@echo 'Invoking: GNU ARM C Compiler'
	arm-none-eabi-gcc -g -gdwarf-2 -mcpu=cortex-m0plus -mthumb -std=c99 '-DDEBUG_EFM=1' '-DDEBUG=1' '-DEFM32HG322F64=1' -I"/home/brecht/Programs/SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v2.4//platform/CMSIS/Include" -I"/home/brecht/Programs/SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v2.4//hardware/kit/common/bsp" -I"/home/brecht/Programs/SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v2.4//platform/Device/SiliconLabs/EFM32HG/Include" -I"/home/brecht/Programs/SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v2.4//hardware/kit/SLSTK3400A_EFM32HG/config" -I"/home/brecht/Programs/SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v2.4//platform/emlib/inc" -I/home/brecht/Qsync/Documents/1-Github-Repositories/dbprint/dbprint-inc -O0 -Wall -c -fmessage-length=0 -mno-sched-prolog -fno-builtin -ffunction-sections -fdata-sections -MMD -MP -MF"src/main.d" -MT"src/main.o" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/udelay.o: ../src/udelay.c
	@echo 'Building file: $<'
	@echo 'Invoking: GNU ARM C Compiler'
	arm-none-eabi-gcc -g -gdwarf-2 -mcpu=cortex-m0plus -mthumb -std=c99 '-DDEBUG_EFM=1' '-DDEBUG=1' '-DEFM32HG322F64=1' -I"/home/brecht/Programs/SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v2.4//platform/CMSIS/Include" -I"/home/brecht/Programs/SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v2.4//hardware/kit/common/bsp" -I"/home/brecht/Programs/SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v2.4//platform/Device/SiliconLabs/EFM32HG/Include" -I"/home/brecht/Programs/SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v2.4//hardware/kit/SLSTK3400A_EFM32HG/config" -I"/home/brecht/Programs/SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v2.4//platform/emlib/inc" -I/home/brecht/Qsync/Documents/1-Github-Repositories/dbprint/dbprint-inc -O0 -Wall -c -fmessage-length=0 -mno-sched-prolog -fno-builtin -ffunction-sections -fdata-sections -MMD -MP -MF"src/udelay.d" -MT"src/udelay.o" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/util.o: ../src/util.c
	@echo 'Building file: $<'
	@echo 'Invoking: GNU ARM C Compiler'
	arm-none-eabi-gcc -g -gdwarf-2 -mcpu=cortex-m0plus -mthumb -std=c99 '-DDEBUG_EFM=1' '-DDEBUG=1' '-DEFM32HG322F64=1' -I"/home/brecht/Programs/SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v2.4//platform/CMSIS/Include" -I"/home/brecht/Programs/SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v2.4//hardware/kit/common/bsp" -I"/home/brecht/Programs/SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v2.4//platform/Device/SiliconLabs/EFM32HG/Include" -I"/home/brecht/Programs/SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v2.4//hardware/kit/SLSTK3400A_EFM32HG/config" -I"/home/brecht/Programs/SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v2.4//platform/emlib/inc" -I/home/brecht/Qsync/Documents/1-Github-Repositories/dbprint/dbprint-inc -O0 -Wall -c -fmessage-length=0 -mno-sched-prolog -fno-builtin -ffunction-sections -fdata-sections -MMD -MP -MF"src/util.d" -MT"src/util.o" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


