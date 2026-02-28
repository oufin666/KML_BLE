################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/BSP/GH3220/Kernel_code/kernel/gh_demo.c \
../Drivers/BSP/GH3220/Kernel_code/kernel/gh_demo_hook.c \
../Drivers/BSP/GH3220/Kernel_code/kernel/gh_demo_protocol.c \
../Drivers/BSP/GH3220/Kernel_code/kernel/gh_demo_reg_array.c \
../Drivers/BSP/GH3220/Kernel_code/kernel/gh_demo_user.c 

OBJS += \
./Drivers/BSP/GH3220/Kernel_code/kernel/gh_demo.o \
./Drivers/BSP/GH3220/Kernel_code/kernel/gh_demo_hook.o \
./Drivers/BSP/GH3220/Kernel_code/kernel/gh_demo_protocol.o \
./Drivers/BSP/GH3220/Kernel_code/kernel/gh_demo_reg_array.o \
./Drivers/BSP/GH3220/Kernel_code/kernel/gh_demo_user.o 

C_DEPS += \
./Drivers/BSP/GH3220/Kernel_code/kernel/gh_demo.d \
./Drivers/BSP/GH3220/Kernel_code/kernel/gh_demo_hook.d \
./Drivers/BSP/GH3220/Kernel_code/kernel/gh_demo_protocol.d \
./Drivers/BSP/GH3220/Kernel_code/kernel/gh_demo_reg_array.d \
./Drivers/BSP/GH3220/Kernel_code/kernel/gh_demo_user.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/BSP/GH3220/Kernel_code/kernel/%.o Drivers/BSP/GH3220/Kernel_code/kernel/%.su Drivers/BSP/GH3220/Kernel_code/kernel/%.cyclo: ../Drivers/BSP/GH3220/Kernel_code/kernel/%.c Drivers/BSP/GH3220/Kernel_code/kernel/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F412Cx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../Drivers/BSP/inc -I../Middlewares/LVGL/GUI -I../Middlewares/LVGL/GUI/lvgl -I../Middlewares/LVGL/GUI/src -I../Middlewares/LVGL/GUI/lvgl/examples/porting -I../FATFS/Target -I../FATFS/App -I../Middlewares/Third_Party/FatFs/src -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-BSP-2f-GH3220-2f-Kernel_code-2f-kernel

clean-Drivers-2f-BSP-2f-GH3220-2f-Kernel_code-2f-kernel:
	-$(RM) ./Drivers/BSP/GH3220/Kernel_code/kernel/gh_demo.cyclo ./Drivers/BSP/GH3220/Kernel_code/kernel/gh_demo.d ./Drivers/BSP/GH3220/Kernel_code/kernel/gh_demo.o ./Drivers/BSP/GH3220/Kernel_code/kernel/gh_demo.su ./Drivers/BSP/GH3220/Kernel_code/kernel/gh_demo_hook.cyclo ./Drivers/BSP/GH3220/Kernel_code/kernel/gh_demo_hook.d ./Drivers/BSP/GH3220/Kernel_code/kernel/gh_demo_hook.o ./Drivers/BSP/GH3220/Kernel_code/kernel/gh_demo_hook.su ./Drivers/BSP/GH3220/Kernel_code/kernel/gh_demo_protocol.cyclo ./Drivers/BSP/GH3220/Kernel_code/kernel/gh_demo_protocol.d ./Drivers/BSP/GH3220/Kernel_code/kernel/gh_demo_protocol.o ./Drivers/BSP/GH3220/Kernel_code/kernel/gh_demo_protocol.su ./Drivers/BSP/GH3220/Kernel_code/kernel/gh_demo_reg_array.cyclo ./Drivers/BSP/GH3220/Kernel_code/kernel/gh_demo_reg_array.d ./Drivers/BSP/GH3220/Kernel_code/kernel/gh_demo_reg_array.o ./Drivers/BSP/GH3220/Kernel_code/kernel/gh_demo_reg_array.su ./Drivers/BSP/GH3220/Kernel_code/kernel/gh_demo_user.cyclo ./Drivers/BSP/GH3220/Kernel_code/kernel/gh_demo_user.d ./Drivers/BSP/GH3220/Kernel_code/kernel/gh_demo_user.o ./Drivers/BSP/GH3220/Kernel_code/kernel/gh_demo_user.su

.PHONY: clean-Drivers-2f-BSP-2f-GH3220-2f-Kernel_code-2f-kernel

