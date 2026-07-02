################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/GY273/gy273.c 

OBJS += \
./Core/Src/GY273/gy273.o 

C_DEPS += \
./Core/Src/GY273/gy273.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/GY273/%.o Core/Src/GY273/%.su Core/Src/GY273/%.cyclo: ../Core/Src/GY273/%.c Core/Src/GY273/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F411xE -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@"  -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-GY273

clean-Core-2f-Src-2f-GY273:
	-$(RM) ./Core/Src/GY273/gy273.cyclo ./Core/Src/GY273/gy273.d ./Core/Src/GY273/gy273.o ./Core/Src/GY273/gy273.su

.PHONY: clean-Core-2f-Src-2f-GY273

