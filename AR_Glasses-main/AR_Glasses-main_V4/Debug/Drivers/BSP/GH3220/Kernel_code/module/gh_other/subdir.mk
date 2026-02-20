################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/BSP/GH3220/Kernel_code/module/gh_other/gh_changeinttime.c \
../Drivers/BSP/GH3220/Kernel_code/module/gh_other/gh_movedetect.c 

C_DEPS += \
./Drivers/BSP/GH3220/Kernel_code/module/gh_other/gh_changeinttime.d \
./Drivers/BSP/GH3220/Kernel_code/module/gh_other/gh_movedetect.d 

OBJS += \
./Drivers/BSP/GH3220/Kernel_code/module/gh_other/gh_changeinttime.o \
./Drivers/BSP/GH3220/Kernel_code/module/gh_other/gh_movedetect.o 


# Each subdirectory must supply rules for building sources it contributes
Drivers/BSP/GH3220/Kernel_code/module/gh_other/%.o Drivers/BSP/GH3220/Kernel_code/module/gh_other/%.su Drivers/BSP/GH3220/Kernel_code/module/gh_other/%.cyclo: ../Drivers/BSP/GH3220/Kernel_code/module/gh_other/%.c Drivers/BSP/GH3220/Kernel_code/module/gh_other/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F412Cx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../Drivers/BSP/inc -I../Drivers/BSP/src -I../Drivers/BSP/inc/kml_utils.h -I../Middlewares/LVGL/GUI -I../Middlewares/LVGL/GUI/lvgl -I../Middlewares/LVGL/GUI/src -I../Middlewares/LVGL/GUI/lvgl/examples/porting -I../FATFS/Target -I../FATFS/App -I../Middlewares/Third_Party/FatFs/src -I../Drivers/BSP/GH3220/Kernel_code/driver/inc -I../Drivers/BSP/GH3220/Kernel_code/kernel/inc -I../Drivers/BSP/GH3220/Kernel_code/module/gh_agc -I../Drivers/BSP/GH3220/Kernel_code/module/gh_ecg -I../Drivers/BSP/GH3220/Kernel_code/module/gh_other -I../Drivers/BSP/GH3220/Kernel_code/module/gh_protocol -I../Drivers/BSP/GH3220/Kernel_code/module/gh_soft_adt -I../Drivers/BSP/GH3220/Algo_code/goodix_algo_application/inc -I../Drivers/BSP/GH3220/Algo_code/goodix_algo_call/inc -I../Drivers/BSP/GH3220/Algo_code/goodix_algo_call/inc/hr_exc -I../Drivers/BSP/GH3220/Algo_code/goodix_algo_call/inc/spo2_pre_exc -I../Core/USER/APP/inc -I../Core/USER/APP/Pages/inc -I../Core/USER/Utils/lv_ext/inc -I../Core/USER/APP/Pages/Startup/inc -I../Core/USER/APP/Pages/Template/inc -I../Core/USER/APP/Pages/HeartRate/inc -I../Core/USER/Utils/Track_Parse/inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-BSP-2f-GH3220-2f-Kernel_code-2f-module-2f-gh_other

clean-Drivers-2f-BSP-2f-GH3220-2f-Kernel_code-2f-module-2f-gh_other:
	-$(RM) ./Drivers/BSP/GH3220/Kernel_code/module/gh_other/gh_changeinttime.cyclo ./Drivers/BSP/GH3220/Kernel_code/module/gh_other/gh_changeinttime.d ./Drivers/BSP/GH3220/Kernel_code/module/gh_other/gh_changeinttime.o ./Drivers/BSP/GH3220/Kernel_code/module/gh_other/gh_changeinttime.su ./Drivers/BSP/GH3220/Kernel_code/module/gh_other/gh_movedetect.cyclo ./Drivers/BSP/GH3220/Kernel_code/module/gh_other/gh_movedetect.d ./Drivers/BSP/GH3220/Kernel_code/module/gh_other/gh_movedetect.o ./Drivers/BSP/GH3220/Kernel_code/module/gh_other/gh_movedetect.su

.PHONY: clean-Drivers-2f-BSP-2f-GH3220-2f-Kernel_code-2f-module-2f-gh_other

