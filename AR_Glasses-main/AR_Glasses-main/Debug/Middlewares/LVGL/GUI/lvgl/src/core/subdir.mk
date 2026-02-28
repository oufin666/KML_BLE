################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Middlewares/LVGL/GUI/lvgl/src/core/lv_disp.c \
../Middlewares/LVGL/GUI/lvgl/src/core/lv_event.c \
../Middlewares/LVGL/GUI/lvgl/src/core/lv_group.c \
../Middlewares/LVGL/GUI/lvgl/src/core/lv_indev.c \
../Middlewares/LVGL/GUI/lvgl/src/core/lv_indev_scroll.c \
../Middlewares/LVGL/GUI/lvgl/src/core/lv_obj.c \
../Middlewares/LVGL/GUI/lvgl/src/core/lv_obj_class.c \
../Middlewares/LVGL/GUI/lvgl/src/core/lv_obj_draw.c \
../Middlewares/LVGL/GUI/lvgl/src/core/lv_obj_pos.c \
../Middlewares/LVGL/GUI/lvgl/src/core/lv_obj_scroll.c \
../Middlewares/LVGL/GUI/lvgl/src/core/lv_obj_style.c \
../Middlewares/LVGL/GUI/lvgl/src/core/lv_obj_style_gen.c \
../Middlewares/LVGL/GUI/lvgl/src/core/lv_obj_tree.c \
../Middlewares/LVGL/GUI/lvgl/src/core/lv_refr.c \
../Middlewares/LVGL/GUI/lvgl/src/core/lv_theme.c 

C_DEPS += \
./Middlewares/LVGL/GUI/lvgl/src/core/lv_disp.d \
./Middlewares/LVGL/GUI/lvgl/src/core/lv_event.d \
./Middlewares/LVGL/GUI/lvgl/src/core/lv_group.d \
./Middlewares/LVGL/GUI/lvgl/src/core/lv_indev.d \
./Middlewares/LVGL/GUI/lvgl/src/core/lv_indev_scroll.d \
./Middlewares/LVGL/GUI/lvgl/src/core/lv_obj.d \
./Middlewares/LVGL/GUI/lvgl/src/core/lv_obj_class.d \
./Middlewares/LVGL/GUI/lvgl/src/core/lv_obj_draw.d \
./Middlewares/LVGL/GUI/lvgl/src/core/lv_obj_pos.d \
./Middlewares/LVGL/GUI/lvgl/src/core/lv_obj_scroll.d \
./Middlewares/LVGL/GUI/lvgl/src/core/lv_obj_style.d \
./Middlewares/LVGL/GUI/lvgl/src/core/lv_obj_style_gen.d \
./Middlewares/LVGL/GUI/lvgl/src/core/lv_obj_tree.d \
./Middlewares/LVGL/GUI/lvgl/src/core/lv_refr.d \
./Middlewares/LVGL/GUI/lvgl/src/core/lv_theme.d 

OBJS += \
./Middlewares/LVGL/GUI/lvgl/src/core/lv_disp.o \
./Middlewares/LVGL/GUI/lvgl/src/core/lv_event.o \
./Middlewares/LVGL/GUI/lvgl/src/core/lv_group.o \
./Middlewares/LVGL/GUI/lvgl/src/core/lv_indev.o \
./Middlewares/LVGL/GUI/lvgl/src/core/lv_indev_scroll.o \
./Middlewares/LVGL/GUI/lvgl/src/core/lv_obj.o \
./Middlewares/LVGL/GUI/lvgl/src/core/lv_obj_class.o \
./Middlewares/LVGL/GUI/lvgl/src/core/lv_obj_draw.o \
./Middlewares/LVGL/GUI/lvgl/src/core/lv_obj_pos.o \
./Middlewares/LVGL/GUI/lvgl/src/core/lv_obj_scroll.o \
./Middlewares/LVGL/GUI/lvgl/src/core/lv_obj_style.o \
./Middlewares/LVGL/GUI/lvgl/src/core/lv_obj_style_gen.o \
./Middlewares/LVGL/GUI/lvgl/src/core/lv_obj_tree.o \
./Middlewares/LVGL/GUI/lvgl/src/core/lv_refr.o \
./Middlewares/LVGL/GUI/lvgl/src/core/lv_theme.o 


# Each subdirectory must supply rules for building sources it contributes
Middlewares/LVGL/GUI/lvgl/src/core/%.o Middlewares/LVGL/GUI/lvgl/src/core/%.su Middlewares/LVGL/GUI/lvgl/src/core/%.cyclo: ../Middlewares/LVGL/GUI/lvgl/src/core/%.c Middlewares/LVGL/GUI/lvgl/src/core/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F412Cx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../Drivers/BSP/inc -I../Drivers/BSP/src -I../Drivers/BSP/inc/kml_utils.h -I../Middlewares/LVGL/GUI -I../Middlewares/LVGL/GUI/lvgl -I../Middlewares/LVGL/GUI/src -I../Middlewares/LVGL/GUI/lvgl/examples/porting -I../FATFS/Target -I../FATFS/App -I../Middlewares/Third_Party/FatFs/src -I../Drivers/BSP/GH3220/Kernel_code/driver/inc -I../Drivers/BSP/GH3220/Kernel_code/kernel/inc -I../Drivers/BSP/GH3220/Kernel_code/module/gh_agc -I../Drivers/BSP/GH3220/Kernel_code/module/gh_ecg -I../Drivers/BSP/GH3220/Kernel_code/module/gh_other -I../Drivers/BSP/GH3220/Kernel_code/module/gh_protocol -I../Drivers/BSP/GH3220/Kernel_code/module/gh_soft_adt -I../Drivers/BSP/GH3220/Algo_code/goodix_algo_application/inc -I../Drivers/BSP/GH3220/Algo_code/goodix_algo_call/inc -I../Drivers/BSP/GH3220/Algo_code/goodix_algo_call/inc/hr_exc -I../Drivers/BSP/GH3220/Algo_code/goodix_algo_call/inc/spo2_pre_exc -I../Core/USER/APP/inc -I../Core/USER/APP/Pages/inc -I../Core/USER/Utils/lv_ext/inc -I../Core/USER/APP/Pages/Startup/inc -I../Core/USER/APP/Pages/Template/inc -I../Core/USER/APP/Pages/HeartRate/inc -I../Core/USER/Utils/Track_Parse/inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Middlewares-2f-LVGL-2f-GUI-2f-lvgl-2f-src-2f-core

clean-Middlewares-2f-LVGL-2f-GUI-2f-lvgl-2f-src-2f-core:
	-$(RM) ./Middlewares/LVGL/GUI/lvgl/src/core/lv_disp.cyclo ./Middlewares/LVGL/GUI/lvgl/src/core/lv_disp.d ./Middlewares/LVGL/GUI/lvgl/src/core/lv_disp.o ./Middlewares/LVGL/GUI/lvgl/src/core/lv_disp.su ./Middlewares/LVGL/GUI/lvgl/src/core/lv_event.cyclo ./Middlewares/LVGL/GUI/lvgl/src/core/lv_event.d ./Middlewares/LVGL/GUI/lvgl/src/core/lv_event.o ./Middlewares/LVGL/GUI/lvgl/src/core/lv_event.su ./Middlewares/LVGL/GUI/lvgl/src/core/lv_group.cyclo ./Middlewares/LVGL/GUI/lvgl/src/core/lv_group.d ./Middlewares/LVGL/GUI/lvgl/src/core/lv_group.o ./Middlewares/LVGL/GUI/lvgl/src/core/lv_group.su ./Middlewares/LVGL/GUI/lvgl/src/core/lv_indev.cyclo ./Middlewares/LVGL/GUI/lvgl/src/core/lv_indev.d ./Middlewares/LVGL/GUI/lvgl/src/core/lv_indev.o ./Middlewares/LVGL/GUI/lvgl/src/core/lv_indev.su ./Middlewares/LVGL/GUI/lvgl/src/core/lv_indev_scroll.cyclo ./Middlewares/LVGL/GUI/lvgl/src/core/lv_indev_scroll.d ./Middlewares/LVGL/GUI/lvgl/src/core/lv_indev_scroll.o ./Middlewares/LVGL/GUI/lvgl/src/core/lv_indev_scroll.su ./Middlewares/LVGL/GUI/lvgl/src/core/lv_obj.cyclo ./Middlewares/LVGL/GUI/lvgl/src/core/lv_obj.d ./Middlewares/LVGL/GUI/lvgl/src/core/lv_obj.o ./Middlewares/LVGL/GUI/lvgl/src/core/lv_obj.su ./Middlewares/LVGL/GUI/lvgl/src/core/lv_obj_class.cyclo ./Middlewares/LVGL/GUI/lvgl/src/core/lv_obj_class.d ./Middlewares/LVGL/GUI/lvgl/src/core/lv_obj_class.o ./Middlewares/LVGL/GUI/lvgl/src/core/lv_obj_class.su ./Middlewares/LVGL/GUI/lvgl/src/core/lv_obj_draw.cyclo ./Middlewares/LVGL/GUI/lvgl/src/core/lv_obj_draw.d ./Middlewares/LVGL/GUI/lvgl/src/core/lv_obj_draw.o ./Middlewares/LVGL/GUI/lvgl/src/core/lv_obj_draw.su ./Middlewares/LVGL/GUI/lvgl/src/core/lv_obj_pos.cyclo ./Middlewares/LVGL/GUI/lvgl/src/core/lv_obj_pos.d ./Middlewares/LVGL/GUI/lvgl/src/core/lv_obj_pos.o ./Middlewares/LVGL/GUI/lvgl/src/core/lv_obj_pos.su ./Middlewares/LVGL/GUI/lvgl/src/core/lv_obj_scroll.cyclo ./Middlewares/LVGL/GUI/lvgl/src/core/lv_obj_scroll.d ./Middlewares/LVGL/GUI/lvgl/src/core/lv_obj_scroll.o ./Middlewares/LVGL/GUI/lvgl/src/core/lv_obj_scroll.su ./Middlewares/LVGL/GUI/lvgl/src/core/lv_obj_style.cyclo ./Middlewares/LVGL/GUI/lvgl/src/core/lv_obj_style.d ./Middlewares/LVGL/GUI/lvgl/src/core/lv_obj_style.o ./Middlewares/LVGL/GUI/lvgl/src/core/lv_obj_style.su ./Middlewares/LVGL/GUI/lvgl/src/core/lv_obj_style_gen.cyclo ./Middlewares/LVGL/GUI/lvgl/src/core/lv_obj_style_gen.d ./Middlewares/LVGL/GUI/lvgl/src/core/lv_obj_style_gen.o ./Middlewares/LVGL/GUI/lvgl/src/core/lv_obj_style_gen.su ./Middlewares/LVGL/GUI/lvgl/src/core/lv_obj_tree.cyclo ./Middlewares/LVGL/GUI/lvgl/src/core/lv_obj_tree.d ./Middlewares/LVGL/GUI/lvgl/src/core/lv_obj_tree.o ./Middlewares/LVGL/GUI/lvgl/src/core/lv_obj_tree.su ./Middlewares/LVGL/GUI/lvgl/src/core/lv_refr.cyclo ./Middlewares/LVGL/GUI/lvgl/src/core/lv_refr.d ./Middlewares/LVGL/GUI/lvgl/src/core/lv_refr.o ./Middlewares/LVGL/GUI/lvgl/src/core/lv_refr.su ./Middlewares/LVGL/GUI/lvgl/src/core/lv_theme.cyclo ./Middlewares/LVGL/GUI/lvgl/src/core/lv_theme.d ./Middlewares/LVGL/GUI/lvgl/src/core/lv_theme.o ./Middlewares/LVGL/GUI/lvgl/src/core/lv_theme.su

.PHONY: clean-Middlewares-2f-LVGL-2f-GUI-2f-lvgl-2f-src-2f-core

