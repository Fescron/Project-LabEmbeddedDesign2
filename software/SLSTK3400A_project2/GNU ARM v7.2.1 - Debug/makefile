################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/subdir.mk
-include emlib/subdir.mk
-include dbprint-scr/subdir.mk
-include CMSIS/EFM32HG/subdir.mk
-include BSP/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: SLSTK3400A_project2.axf

# Tool invocations
SLSTK3400A_project2.axf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: GNU ARM C Linker'
	arm-none-eabi-gcc -g -gdwarf-2 -mcpu=cortex-m0plus -mthumb -T "SLSTK3400A_project2.ld" -Xlinker --gc-sections -Xlinker -Map="SLSTK3400A_project2.map" --specs=nano.specs -o SLSTK3400A_project2.axf "./src/main.o" "./emlib/em_assert.o" "./emlib/em_cmu.o" "./emlib/em_core.o" "./emlib/em_emu.o" "./emlib/em_gpio.o" "./emlib/em_usart.o" "./CMSIS/EFM32HG/startup_efm32hg.o" "./CMSIS/EFM32HG/system_efm32hg.o" "./BSP/bsp_stk_leds.o" "./dbprint-scr/dbprint.o" "./emlib/em_rtc.o" "./src/ADXL362.o" "./src/DS18B20.o" "./src/driftsensor.o" "./src/handlers.o" "./src/main-driftsensor.o" "./src/udelay.o" "./src/util.o" -Wl,--start-group -lgcc -lc -lnosys -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

	@echo 'Building hex file: SLSTK3400A_project2.hex'
	arm-none-eabi-objcopy -O ihex "SLSTK3400A_project2.axf" "SLSTK3400A_project2.hex"
	@echo ' '

	@echo 'Building bin file: SLSTK3400A_project2.bin'
	arm-none-eabi-objcopy -O binary "SLSTK3400A_project2.axf" "SLSTK3400A_project2.bin"
	@echo ' '

	@echo 'Building s37 file: SLSTK3400A_project2.s37'
	arm-none-eabi-objcopy -O srec "SLSTK3400A_project2.axf" "SLSTK3400A_project2.s37"
	@echo ' '

	@echo 'Running size tool'
	arm-none-eabi-size "SLSTK3400A_project2.axf"
	@echo ' '

# Other Targets
clean:
	-$(RM) $(EXECUTABLES)$(OBJS)$(C_DEPS) SLSTK3400A_project2.axf
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
