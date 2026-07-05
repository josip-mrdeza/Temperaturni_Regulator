################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/Sensors/dht22.c 

OBJS += \
./Core/Src/Sensors/dht22.o 

C_DEPS += \
./Core/Src/Sensors/dht22.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/Sensors/%.o Core/Src/Sensors/%.su Core/Src/Sensors/%.cyclo: ../Core/Src/Sensors/%.c Core/Src/Sensors/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F411xE -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../USB_DEVICE/App -I../USB_DEVICE/Target -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@"  -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-Sensors

clean-Core-2f-Src-2f-Sensors:
	-$(RM) ./Core/Src/Sensors/dht22.cyclo ./Core/Src/Sensors/dht22.d ./Core/Src/Sensors/dht22.o ./Core/Src/Sensors/dht22.su

.PHONY: clean-Core-2f-Src-2f-Sensors

