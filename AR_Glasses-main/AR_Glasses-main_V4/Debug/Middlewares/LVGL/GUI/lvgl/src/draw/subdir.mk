################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Middlewares/LVGL/GUI/lvgl/src/draw/lv_draw.c \
../Middlewares/LVGL/GUI/lvgl/src/draw/lv_draw_arc.c \
../Middlewares/LVGL/GUI/lvgl/src/draw/lv_draw_img.c \
../Middlewares/LVGL/GUI/lvgl/src/draw/lv_draw_label.c \
../Middlewares/LVGL/GUI/lvgl/src/draw/lv_draw_line.c \
../Middlewares/LVGL/GUI/lvgl/src/draw/lv_draw_mask.c \
../Middlewares/LVGL/GUI/lvgl/src/draw/lv_draw_rect.c \
../Middlewares/LVGL/GUI/lvgl/src/draw/lv_draw_triangle.c \
../Middlewares/LVGL/GUI/lvgl/src/draw/lv_img_buf.c \
../Middlewares/LVGL/GUI/lvgl/src/draw/lv_img_cache.c \
../Middlewares/LVGL/GUI/lvgl/src/draw/lv_img_decoder.c 

C_DEPS += \
./Middlewares/LVGL/GUI/lvgl/src/draw/lv_draw.d \
./Middlewares/LVGL/GUI/lvgl/src/draw/lv_draw_arc.d \
./Middlewares/LVGL/GUI/lvgl/src/draw/lv_draw_img.d \
./Middlewares/LVGL/GUI/lvgl/src/draw/lv_draw_label.d \
./Middlewares/LVGL/GUI/lvgl/src/draw/lv_draw_line.d \
./Middlewares/LVGL/GUI/lvgl/src/draw/lv_draw_mask.d \
./Middlewares/LVGL/GUI/lvgl/src/draw/lv_draw_rect.d \
./Middlewares/LVGL/GUI/lvgl/src/draw/lv_draw_triangle.d \
./Middlewares/LVGL/GUI/lvgl/src/draw/lv_img_buf.d \
./Middlewares/LVGL/GUI/lvgl/src/draw/lv_img_cache.d \
./Middlewares/LVGL/GUI/lvgl/src/draw/lv_img_decoder.d 

OBJS += \
./Middlewares/LVGL/GUI/lvgl/src/draw/lv_draw.o \
./Middlewares/LVGL/GUI/lvgl/src/draw/lv_draw_arc.o \
./Middlewares/LVGL/GUI/lvgl/src/draw/lv_draw_img.o \
./Middlewares/LVGL/GUI/lvgl/src/draw/lv_draw_label.o \
./Middlewares/LVGL/GUI/lvgl/src/draw/lv_draw_line.o \
./Middlewares/LVGL/GUI/lvgl/src/draw/lv_draw_mask.o \
./Middlewares/LVGL/GUI/lvgl/src/draw/lv_draw_rect.o \
./Middlewares/LVGL/GUI/lvgl/src/draw/lv_draw_triangle.o \
./Middlewares/LVGL/GUI/lvgl/src/draw/lv_img_buf.o \
./Middlewares/LVGL/GUI/lvgl/src/draw/lv_img_cache.o \
./Middlewares/LVGL/GUI/lvgl/src/draw/lv_img_decoder.o 


# Each subdirectory must supply rules for building sources it contributes
Middlewares/LVGL/GUI/lvgl/src/draw/%.o Middlewares/LVGL/GUI/lvgl/src/draw/%.su Middlewares/LVGL/GUI/lvgl/src/draw/%.cyclo: ../Middlewares/LVGL/GUI/lvgl/src/draw/%.c Middlewares/LVGL/GUI/lvgl/src/draw/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F412Cx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../Drivers/BSP/inc -I../Drivers/BSP/src -I../Drivers/BSP/inc/kml_utils.h -I../Middlewares/LVGL/GUI -I../Middlewares/LVGL/GUI/lvgl -I../Middlewares/LVGL/GUI/src -I../Middlewares/LVGL/GUI/lvgl/examples/porting -I../FATFS/Target -I../FATFS/App -I../Middlewares/Third_Party/FatFs/src -I../Drivers/BSP/GH3220/Kernel_code/driver/inc -I../Drivers/BSP/GH3220/Kernel_code/kernel/inc -I../Drivers/BSP/GH3220/Kernel_code/module/gh_agc -I../Drivers/BSP/GH3220/Kernel_code/module/gh_ecg -I../Drivers/BSP/GH3220/Kernel_code/module/gh_other -I../Drivers/BSP/GH3220/Kernel_code/module/gh_protocol -I../Drivers/BSP/GH3220/Kernel_code/module/gh_soft_adt -I../Drivers/BSP/GH3220/Algo_code/goodix_algo_application/inc -I../Drivers/BSP/GH3220/Algo_code/goodix_algo_call/inc -I../Drivers/BSP/GH3220/Algo_code/goodix_algo_call/inc/hr_exc -I../Drivers/BSP/GH3220/Algo_code/goodix_algo_call/inc/spo2_pre_exc -I../Core/USER/APP/inc -I../Core/USER/APP/Pages/inc -I../Core/USER/Utils/lv_ext/inc -I../Core/USER/APP/Pages/Startup/inc -I../Core/USER/APP/Pages/Template/inc -I../Core/USER/APP/Pages/HeartRate/inc -I../Core/USER/Utils/Track_Parse/inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Middlewares-2f-LVGL-2f-GUI-2f-lvgl-2f-src-2f-draw

clean-Middlewares-2f-LVGL-2f-GUI-2f-lvgl-2f-src-2f-draw:
	-$(RM) ./Middlewares/LVGL/GUI/lvgl/src/draw/lv_draw.cyclo ./Middlewares/LVGL/GUI/lvgl/src/draw/lv_draw.d ./Middlewares/LVGL/GUI/lvgl/src/draw/lv_draw.o ./Middlewares/LVGL/GUI/lvgl/src/draw/lv_draw.su ./Middlewares/LVGL/GUI/lvgl/src/draw/lv_draw_arc.cyclo ./Middlewares/LVGL/GUI/lvgl/src/draw/lv_draw_arc.d ./Middlewares/LVGL/GUI/lvgl/src/draw/lv_draw_arc.o ./Middlewares/LVGL/GUI/lvgl/src/draw/lv_draw_arc.su ./Middlewares/LVGL/GUI/lvgl/src/draw/lv_draw_img.cyclo ./Middlewares/LVGL/GUI/lvgl/src/draw/lv_draw_img.d ./Middlewares/LVGL/GUI/lvgl/src/draw/lv_draw_img.o ./Middlewares/LVGL/GUI/lvgl/src/draw/lv_draw_img.su ./Middlewares/LVGL/GUI/lvgl/src/draw/lv_draw_label.cyclo ./Middlewares/LVGL/GUI/lvgl/src/draw/lv_draw_label.d ./Middlewares/LVGL/GUI/lvgl/src/draw/lv_draw_label.o ./Middlewares/LVGL/GUI/lvgl/src/draw/lv_draw_label.su ./Middlewares/LVGL/GUI/lvgl/src/draw/lv_draw_line.cyclo ./Middlewares/LVGL/GUI/lvgl/src/draw/lv_draw_line.d ./Middlewares/LVGL/GUI/lvgl/src/draw/lv_draw_line.o ./Middlewares/LVGL/GUI/lvgl/src/draw/lv_draw_line.su ./Middlewares/LVGL/GUI/lvgl/src/draw/lv_draw_mask.cyclo ./Middlewares/LVGL/GUI/lvgl/src/draw/lv_draw_mask.d ./Middlewares/LVGL/GUI/lvgl/src/draw/lv_draw_mask.o ./Middlewares/LVGL/GUI/lvgl/src/draw/lv_draw_mask.su ./Middlewares/LVGL/GUI/lvgl/src/draw/lv_draw_rect.cyclo ./Middlewares/LVGL/GUI/lvgl/src/draw/lv_draw_rect.d ./Middlewares/LVGL/GUI/lvgl/src/draw/lv_draw_rect.o ./Middlewares/LVGL/GUI/lvgl/src/draw/lv_draw_rect.su ./Middlewares/LVGL/GUI/lvgl/src/draw/lv_draw_triangle.cyclo ./Middlewares/LVGL/GUI/lvgl/src/draw/lv_draw_triangle.d ./Middlewares/LVGL/GUI/lvgl/src/draw/lv_draw_triangle.o ./Middlewares/LVGL/GUI/lvgl/src/draw/lv_draw_triangle.su ./Middlewares/LVGL/GUI/lvgl/src/draw/lv_img_buf.cyclo ./Middlewares/LVGL/GUI/lvgl/src/draw/lv_img_buf.d ./Middlewares/LVGL/GUI/lvgl/src/draw/lv_img_buf.o ./Middlewares/LVGL/GUI/lvgl/src/draw/lv_img_buf.su ./Middlewares/LVGL/GUI/lvgl/src/draw/lv_img_cache.cyclo ./Middlewares/LVGL/GUI/lvgl/src/draw/lv_img_cache.d ./Middlewares/LVGL/GUI/lvgl/src/draw/lv_img_cache.o ./Middlewares/LVGL/GUI/lvgl/src/draw/lv_img_cache.su ./Middlewares/LVGL/GUI/lvgl/src/draw/lv_img_decoder.cyclo ./Middlewares/LVGL/GUI/lvgl/src/draw/lv_img_decoder.d ./Middlewares/LVGL/GUI/lvgl/src/draw/lv_img_decoder.o ./Middlewares/LVGL/GUI/lvgl/src/draw/lv_img_decoder.su

.PHONY: clean-Middlewares-2f-LVGL-2f-GUI-2f-lvgl-2f-src-2f-draw

