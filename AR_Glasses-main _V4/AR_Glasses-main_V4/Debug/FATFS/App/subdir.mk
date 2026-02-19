################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../FATFS/App/fatfs.c 

C_DEPS += \
./FATFS/App/fatfs.d 

OBJS += \
./FATFS/App/fatfs.o 


# Each subdirectory must supply rules for building sources it contributes
FATFS/App/%.o FATFS/App/%.su FATFS/App/%.cyclo: ../FATFS/App/%.c FATFS/App/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F412Cx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../Drivers/BSP/inc -I../Middlewares/LVGL/GUI -I../Middlewares/LVGL/GUI/lvgl -I../Middlewares/LVGL/GUI/src -I../Middlewares/LVGL/GUI/lvgl/examples/porting -I../FATFS/Target -I../FATFS/App -I../Middlewares/Third_Party/FatFs/src -I../Drivers/BSP/GH3220/Kernel_code/driver/inc -I../Drivers/BSP/GH3220/Kernel_code/kernel/inc -I../Drivers/BSP/GH3220/Kernel_code/module/gh_agc -I../Drivers/BSP/GH3220/Kernel_code/module/gh_ecg -I../Drivers/BSP/GH3220/Kernel_code/module/gh_other -I../Drivers/BSP/GH3220/Kernel_code/module/gh_protocol -I../Drivers/BSP/GH3220/Kernel_code/module/gh_soft_adt -I../Drivers/BSP/GH3220/Algo_code/goodix_algo_application/inc -I../Drivers/BSP/GH3220/Algo_code/goodix_algo_call/inc -I../Drivers/BSP/GH3220/Algo_code/goodix_algo_call/inc/hr_exc -I../Drivers/BSP/GH3220/Algo_code/goodix_algo_call/inc/spo2_pre_exc -I../Core/USER/APP/inc -I../Core/USER/APP/Pages/inc -I../Core/USER/Utils/lv_ext/inc -I../Core/USER/APP/Pages/Startup/inc -I../Core/USER/APP/Pages/Template/inc -I../Core/USER/APP/Pages/HeartRate/inc -I../Core/USER/Utils/Track_Parse/inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-FATFS-2f-App

clean-FATFS-2f-App:
	-$(RM) ./FATFS/App/fatfs.cyclo ./FATFS/App/fatfs.d ./FATFS/App/fatfs.o ./FATFS/App/fatfs.su

.PHONY: clean-FATFS-2f-App

