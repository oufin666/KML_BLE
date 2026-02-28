################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Middlewares/LVGL/GUI/lvgl/src/draw/sw/lv_draw_sw.c \
../Middlewares/LVGL/GUI/lvgl/src/draw/sw/lv_draw_sw_arc.c \
../Middlewares/LVGL/GUI/lvgl/src/draw/sw/lv_draw_sw_blend.c \
../Middlewares/LVGL/GUI/lvgl/src/draw/sw/lv_draw_sw_dither.c \
../Middlewares/LVGL/GUI/lvgl/src/draw/sw/lv_draw_sw_gradient.c \
../Middlewares/LVGL/GUI/lvgl/src/draw/sw/lv_draw_sw_img.c \
../Middlewares/LVGL/GUI/lvgl/src/draw/sw/lv_draw_sw_letter.c \
../Middlewares/LVGL/GUI/lvgl/src/draw/sw/lv_draw_sw_line.c \
../Middlewares/LVGL/GUI/lvgl/src/draw/sw/lv_draw_sw_polygon.c \
../Middlewares/LVGL/GUI/lvgl/src/draw/sw/lv_draw_sw_rect.c 

C_DEPS += \
./Middlewares/LVGL/GUI/lvgl/src/draw/sw/lv_draw_sw.d \
./Middlewares/LVGL/GUI/lvgl/src/draw/sw/lv_draw_sw_arc.d \
./Middlewares/LVGL/GUI/lvgl/src/draw/sw/lv_draw_sw_blend.d \
./Middlewares/LVGL/GUI/lvgl/src/draw/sw/lv_draw_sw_dither.d \
./Middlewares/LVGL/GUI/lvgl/src/draw/sw/lv_draw_sw_gradient.d \
./Middlewares/LVGL/GUI/lvgl/src/draw/sw/lv_draw_sw_img.d \
./Middlewares/LVGL/GUI/lvgl/src/draw/sw/lv_draw_sw_letter.d \
./Middlewares/LVGL/GUI/lvgl/src/draw/sw/lv_draw_sw_line.d \
./Middlewares/LVGL/GUI/lvgl/src/draw/sw/lv_draw_sw_polygon.d \
./Middlewares/LVGL/GUI/lvgl/src/draw/sw/lv_draw_sw_rect.d 

OBJS += \
./Middlewares/LVGL/GUI/lvgl/src/draw/sw/lv_draw_sw.o \
./Middlewares/LVGL/GUI/lvgl/src/draw/sw/lv_draw_sw_arc.o \
./Middlewares/LVGL/GUI/lvgl/src/draw/sw/lv_draw_sw_blend.o \
./Middlewares/LVGL/GUI/lvgl/src/draw/sw/lv_draw_sw_dither.o \
./Middlewares/LVGL/GUI/lvgl/src/draw/sw/lv_draw_sw_gradient.o \
./Middlewares/LVGL/GUI/lvgl/src/draw/sw/lv_draw_sw_img.o \
./Middlewares/LVGL/GUI/lvgl/src/draw/sw/lv_draw_sw_letter.o \
./Middlewares/LVGL/GUI/lvgl/src/draw/sw/lv_draw_sw_line.o \
./Middlewares/LVGL/GUI/lvgl/src/draw/sw/lv_draw_sw_polygon.o \
./Middlewares/LVGL/GUI/lvgl/src/draw/sw/lv_draw_sw_rect.o 


# Each subdirectory must supply rules for building sources it contributes
Middlewares/LVGL/GUI/lvgl/src/draw/sw/%.o Middlewares/LVGL/GUI/lvgl/src/draw/sw/%.su Middlewares/LVGL/GUI/lvgl/src/draw/sw/%.cyclo: ../Middlewares/LVGL/GUI/lvgl/src/draw/sw/%.c Middlewares/LVGL/GUI/lvgl/src/draw/sw/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F412Cx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../Drivers/BSP/inc -I../Drivers/BSP/src -I../Drivers/BSP/inc/kml_utils.h -I../Middlewares/LVGL/GUI -I../Middlewares/LVGL/GUI/lvgl -I../Middlewares/LVGL/GUI/src -I../Middlewares/LVGL/GUI/lvgl/examples/porting -I../FATFS/Target -I../FATFS/App -I../Middlewares/Third_Party/FatFs/src -I../Drivers/BSP/GH3220/Kernel_code/driver/inc -I../Drivers/BSP/GH3220/Kernel_code/kernel/inc -I../Drivers/BSP/GH3220/Kernel_code/module/gh_agc -I../Drivers/BSP/GH3220/Kernel_code/module/gh_ecg -I../Drivers/BSP/GH3220/Kernel_code/module/gh_other -I../Drivers/BSP/GH3220/Kernel_code/module/gh_protocol -I../Drivers/BSP/GH3220/Kernel_code/module/gh_soft_adt -I../Drivers/BSP/GH3220/Algo_code/goodix_algo_application/inc -I../Drivers/BSP/GH3220/Algo_code/goodix_algo_call/inc -I../Drivers/BSP/GH3220/Algo_code/goodix_algo_call/inc/hr_exc -I../Drivers/BSP/GH3220/Algo_code/goodix_algo_call/inc/spo2_pre_exc -I../Core/USER/APP/inc -I../Core/USER/APP/Pages/inc -I../Core/USER/Utils/lv_ext/inc -I../Core/USER/APP/Pages/Startup/inc -I../Core/USER/APP/Pages/Template/inc -I../Core/USER/APP/Pages/HeartRate/inc -I../Core/USER/Utils/Track_Parse/inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Middlewares-2f-LVGL-2f-GUI-2f-lvgl-2f-src-2f-draw-2f-sw

clean-Middlewares-2f-LVGL-2f-GUI-2f-lvgl-2f-src-2f-draw-2f-sw:
	-$(RM) ./Middlewares/LVGL/GUI/lvgl/src/draw/sw/lv_draw_sw.cyclo ./Middlewares/LVGL/GUI/lvgl/src/draw/sw/lv_draw_sw.d ./Middlewares/LVGL/GUI/lvgl/src/draw/sw/lv_draw_sw.o ./Middlewares/LVGL/GUI/lvgl/src/draw/sw/lv_draw_sw.su ./Middlewares/LVGL/GUI/lvgl/src/draw/sw/lv_draw_sw_arc.cyclo ./Middlewares/LVGL/GUI/lvgl/src/draw/sw/lv_draw_sw_arc.d ./Middlewares/LVGL/GUI/lvgl/src/draw/sw/lv_draw_sw_arc.o ./Middlewares/LVGL/GUI/lvgl/src/draw/sw/lv_draw_sw_arc.su ./Middlewares/LVGL/GUI/lvgl/src/draw/sw/lv_draw_sw_blend.cyclo ./Middlewares/LVGL/GUI/lvgl/src/draw/sw/lv_draw_sw_blend.d ./Middlewares/LVGL/GUI/lvgl/src/draw/sw/lv_draw_sw_blend.o ./Middlewares/LVGL/GUI/lvgl/src/draw/sw/lv_draw_sw_blend.su ./Middlewares/LVGL/GUI/lvgl/src/draw/sw/lv_draw_sw_dither.cyclo ./Middlewares/LVGL/GUI/lvgl/src/draw/sw/lv_draw_sw_dither.d ./Middlewares/LVGL/GUI/lvgl/src/draw/sw/lv_draw_sw_dither.o ./Middlewares/LVGL/GUI/lvgl/src/draw/sw/lv_draw_sw_dither.su ./Middlewares/LVGL/GUI/lvgl/src/draw/sw/lv_draw_sw_gradient.cyclo ./Middlewares/LVGL/GUI/lvgl/src/draw/sw/lv_draw_sw_gradient.d ./Middlewares/LVGL/GUI/lvgl/src/draw/sw/lv_draw_sw_gradient.o ./Middlewares/LVGL/GUI/lvgl/src/draw/sw/lv_draw_sw_gradient.su ./Middlewares/LVGL/GUI/lvgl/src/draw/sw/lv_draw_sw_img.cyclo ./Middlewares/LVGL/GUI/lvgl/src/draw/sw/lv_draw_sw_img.d ./Middlewares/LVGL/GUI/lvgl/src/draw/sw/lv_draw_sw_img.o ./Middlewares/LVGL/GUI/lvgl/src/draw/sw/lv_draw_sw_img.su ./Middlewares/LVGL/GUI/lvgl/src/draw/sw/lv_draw_sw_letter.cyclo ./Middlewares/LVGL/GUI/lvgl/src/draw/sw/lv_draw_sw_letter.d ./Middlewares/LVGL/GUI/lvgl/src/draw/sw/lv_draw_sw_letter.o ./Middlewares/LVGL/GUI/lvgl/src/draw/sw/lv_draw_sw_letter.su ./Middlewares/LVGL/GUI/lvgl/src/draw/sw/lv_draw_sw_line.cyclo ./Middlewares/LVGL/GUI/lvgl/src/draw/sw/lv_draw_sw_line.d ./Middlewares/LVGL/GUI/lvgl/src/draw/sw/lv_draw_sw_line.o ./Middlewares/LVGL/GUI/lvgl/src/draw/sw/lv_draw_sw_line.su ./Middlewares/LVGL/GUI/lvgl/src/draw/sw/lv_draw_sw_polygon.cyclo ./Middlewares/LVGL/GUI/lvgl/src/draw/sw/lv_draw_sw_polygon.d ./Middlewares/LVGL/GUI/lvgl/src/draw/sw/lv_draw_sw_polygon.o ./Middlewares/LVGL/GUI/lvgl/src/draw/sw/lv_draw_sw_polygon.su ./Middlewares/LVGL/GUI/lvgl/src/draw/sw/lv_draw_sw_rect.cyclo ./Middlewares/LVGL/GUI/lvgl/src/draw/sw/lv_draw_sw_rect.d ./Middlewares/LVGL/GUI/lvgl/src/draw/sw/lv_draw_sw_rect.o ./Middlewares/LVGL/GUI/lvgl/src/draw/sw/lv_draw_sw_rect.su

.PHONY: clean-Middlewares-2f-LVGL-2f-GUI-2f-lvgl-2f-src-2f-draw-2f-sw

