################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Middlewares/LVGL/GUI/lvgl/src/draw/sdl/lv_draw_sdl.c \
../Middlewares/LVGL/GUI/lvgl/src/draw/sdl/lv_draw_sdl_arc.c \
../Middlewares/LVGL/GUI/lvgl/src/draw/sdl/lv_draw_sdl_bg.c \
../Middlewares/LVGL/GUI/lvgl/src/draw/sdl/lv_draw_sdl_composite.c \
../Middlewares/LVGL/GUI/lvgl/src/draw/sdl/lv_draw_sdl_img.c \
../Middlewares/LVGL/GUI/lvgl/src/draw/sdl/lv_draw_sdl_label.c \
../Middlewares/LVGL/GUI/lvgl/src/draw/sdl/lv_draw_sdl_line.c \
../Middlewares/LVGL/GUI/lvgl/src/draw/sdl/lv_draw_sdl_mask.c \
../Middlewares/LVGL/GUI/lvgl/src/draw/sdl/lv_draw_sdl_polygon.c \
../Middlewares/LVGL/GUI/lvgl/src/draw/sdl/lv_draw_sdl_rect.c \
../Middlewares/LVGL/GUI/lvgl/src/draw/sdl/lv_draw_sdl_stack_blur.c \
../Middlewares/LVGL/GUI/lvgl/src/draw/sdl/lv_draw_sdl_texture_cache.c \
../Middlewares/LVGL/GUI/lvgl/src/draw/sdl/lv_draw_sdl_utils.c 

C_DEPS += \
./Middlewares/LVGL/GUI/lvgl/src/draw/sdl/lv_draw_sdl.d \
./Middlewares/LVGL/GUI/lvgl/src/draw/sdl/lv_draw_sdl_arc.d \
./Middlewares/LVGL/GUI/lvgl/src/draw/sdl/lv_draw_sdl_bg.d \
./Middlewares/LVGL/GUI/lvgl/src/draw/sdl/lv_draw_sdl_composite.d \
./Middlewares/LVGL/GUI/lvgl/src/draw/sdl/lv_draw_sdl_img.d \
./Middlewares/LVGL/GUI/lvgl/src/draw/sdl/lv_draw_sdl_label.d \
./Middlewares/LVGL/GUI/lvgl/src/draw/sdl/lv_draw_sdl_line.d \
./Middlewares/LVGL/GUI/lvgl/src/draw/sdl/lv_draw_sdl_mask.d \
./Middlewares/LVGL/GUI/lvgl/src/draw/sdl/lv_draw_sdl_polygon.d \
./Middlewares/LVGL/GUI/lvgl/src/draw/sdl/lv_draw_sdl_rect.d \
./Middlewares/LVGL/GUI/lvgl/src/draw/sdl/lv_draw_sdl_stack_blur.d \
./Middlewares/LVGL/GUI/lvgl/src/draw/sdl/lv_draw_sdl_texture_cache.d \
./Middlewares/LVGL/GUI/lvgl/src/draw/sdl/lv_draw_sdl_utils.d 

OBJS += \
./Middlewares/LVGL/GUI/lvgl/src/draw/sdl/lv_draw_sdl.o \
./Middlewares/LVGL/GUI/lvgl/src/draw/sdl/lv_draw_sdl_arc.o \
./Middlewares/LVGL/GUI/lvgl/src/draw/sdl/lv_draw_sdl_bg.o \
./Middlewares/LVGL/GUI/lvgl/src/draw/sdl/lv_draw_sdl_composite.o \
./Middlewares/LVGL/GUI/lvgl/src/draw/sdl/lv_draw_sdl_img.o \
./Middlewares/LVGL/GUI/lvgl/src/draw/sdl/lv_draw_sdl_label.o \
./Middlewares/LVGL/GUI/lvgl/src/draw/sdl/lv_draw_sdl_line.o \
./Middlewares/LVGL/GUI/lvgl/src/draw/sdl/lv_draw_sdl_mask.o \
./Middlewares/LVGL/GUI/lvgl/src/draw/sdl/lv_draw_sdl_polygon.o \
./Middlewares/LVGL/GUI/lvgl/src/draw/sdl/lv_draw_sdl_rect.o \
./Middlewares/LVGL/GUI/lvgl/src/draw/sdl/lv_draw_sdl_stack_blur.o \
./Middlewares/LVGL/GUI/lvgl/src/draw/sdl/lv_draw_sdl_texture_cache.o \
./Middlewares/LVGL/GUI/lvgl/src/draw/sdl/lv_draw_sdl_utils.o 


# Each subdirectory must supply rules for building sources it contributes
Middlewares/LVGL/GUI/lvgl/src/draw/sdl/%.o Middlewares/LVGL/GUI/lvgl/src/draw/sdl/%.su Middlewares/LVGL/GUI/lvgl/src/draw/sdl/%.cyclo: ../Middlewares/LVGL/GUI/lvgl/src/draw/sdl/%.c Middlewares/LVGL/GUI/lvgl/src/draw/sdl/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F412Cx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../Drivers/BSP/inc -I../Middlewares/LVGL/GUI -I../Middlewares/LVGL/GUI/lvgl -I../Middlewares/LVGL/GUI/src -I../Middlewares/LVGL/GUI/lvgl/examples/porting -I../FATFS/Target -I../FATFS/App -I../Middlewares/Third_Party/FatFs/src -I../Drivers/BSP/GH3220/Kernel_code/driver/inc -I../Drivers/BSP/GH3220/Kernel_code/kernel/inc -I../Drivers/BSP/GH3220/Kernel_code/module/gh_agc -I../Drivers/BSP/GH3220/Kernel_code/module/gh_ecg -I../Drivers/BSP/GH3220/Kernel_code/module/gh_other -I../Drivers/BSP/GH3220/Kernel_code/module/gh_protocol -I../Drivers/BSP/GH3220/Kernel_code/module/gh_soft_adt -I../Drivers/BSP/GH3220/Algo_code/goodix_algo_application/inc -I../Drivers/BSP/GH3220/Algo_code/goodix_algo_call/inc -I../Drivers/BSP/GH3220/Algo_code/goodix_algo_call/inc/hr_exc -I../Drivers/BSP/GH3220/Algo_code/goodix_algo_call/inc/spo2_pre_exc -I../Core/USER/APP/inc -I../Core/USER/APP/Pages/inc -I../Core/USER/Utils/lv_ext/inc -I../Core/USER/APP/Pages/Startup/inc -I../Core/USER/APP/Pages/Template/inc -I../Core/USER/APP/Pages/HeartRate/inc -I../Core/USER/Utils/Track_Parse/inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Middlewares-2f-LVGL-2f-GUI-2f-lvgl-2f-src-2f-draw-2f-sdl

clean-Middlewares-2f-LVGL-2f-GUI-2f-lvgl-2f-src-2f-draw-2f-sdl:
	-$(RM) ./Middlewares/LVGL/GUI/lvgl/src/draw/sdl/lv_draw_sdl.cyclo ./Middlewares/LVGL/GUI/lvgl/src/draw/sdl/lv_draw_sdl.d ./Middlewares/LVGL/GUI/lvgl/src/draw/sdl/lv_draw_sdl.o ./Middlewares/LVGL/GUI/lvgl/src/draw/sdl/lv_draw_sdl.su ./Middlewares/LVGL/GUI/lvgl/src/draw/sdl/lv_draw_sdl_arc.cyclo ./Middlewares/LVGL/GUI/lvgl/src/draw/sdl/lv_draw_sdl_arc.d ./Middlewares/LVGL/GUI/lvgl/src/draw/sdl/lv_draw_sdl_arc.o ./Middlewares/LVGL/GUI/lvgl/src/draw/sdl/lv_draw_sdl_arc.su ./Middlewares/LVGL/GUI/lvgl/src/draw/sdl/lv_draw_sdl_bg.cyclo ./Middlewares/LVGL/GUI/lvgl/src/draw/sdl/lv_draw_sdl_bg.d ./Middlewares/LVGL/GUI/lvgl/src/draw/sdl/lv_draw_sdl_bg.o ./Middlewares/LVGL/GUI/lvgl/src/draw/sdl/lv_draw_sdl_bg.su ./Middlewares/LVGL/GUI/lvgl/src/draw/sdl/lv_draw_sdl_composite.cyclo ./Middlewares/LVGL/GUI/lvgl/src/draw/sdl/lv_draw_sdl_composite.d ./Middlewares/LVGL/GUI/lvgl/src/draw/sdl/lv_draw_sdl_composite.o ./Middlewares/LVGL/GUI/lvgl/src/draw/sdl/lv_draw_sdl_composite.su ./Middlewares/LVGL/GUI/lvgl/src/draw/sdl/lv_draw_sdl_img.cyclo ./Middlewares/LVGL/GUI/lvgl/src/draw/sdl/lv_draw_sdl_img.d ./Middlewares/LVGL/GUI/lvgl/src/draw/sdl/lv_draw_sdl_img.o ./Middlewares/LVGL/GUI/lvgl/src/draw/sdl/lv_draw_sdl_img.su ./Middlewares/LVGL/GUI/lvgl/src/draw/sdl/lv_draw_sdl_label.cyclo ./Middlewares/LVGL/GUI/lvgl/src/draw/sdl/lv_draw_sdl_label.d ./Middlewares/LVGL/GUI/lvgl/src/draw/sdl/lv_draw_sdl_label.o ./Middlewares/LVGL/GUI/lvgl/src/draw/sdl/lv_draw_sdl_label.su ./Middlewares/LVGL/GUI/lvgl/src/draw/sdl/lv_draw_sdl_line.cyclo ./Middlewares/LVGL/GUI/lvgl/src/draw/sdl/lv_draw_sdl_line.d ./Middlewares/LVGL/GUI/lvgl/src/draw/sdl/lv_draw_sdl_line.o ./Middlewares/LVGL/GUI/lvgl/src/draw/sdl/lv_draw_sdl_line.su ./Middlewares/LVGL/GUI/lvgl/src/draw/sdl/lv_draw_sdl_mask.cyclo ./Middlewares/LVGL/GUI/lvgl/src/draw/sdl/lv_draw_sdl_mask.d ./Middlewares/LVGL/GUI/lvgl/src/draw/sdl/lv_draw_sdl_mask.o ./Middlewares/LVGL/GUI/lvgl/src/draw/sdl/lv_draw_sdl_mask.su ./Middlewares/LVGL/GUI/lvgl/src/draw/sdl/lv_draw_sdl_polygon.cyclo ./Middlewares/LVGL/GUI/lvgl/src/draw/sdl/lv_draw_sdl_polygon.d ./Middlewares/LVGL/GUI/lvgl/src/draw/sdl/lv_draw_sdl_polygon.o ./Middlewares/LVGL/GUI/lvgl/src/draw/sdl/lv_draw_sdl_polygon.su ./Middlewares/LVGL/GUI/lvgl/src/draw/sdl/lv_draw_sdl_rect.cyclo ./Middlewares/LVGL/GUI/lvgl/src/draw/sdl/lv_draw_sdl_rect.d ./Middlewares/LVGL/GUI/lvgl/src/draw/sdl/lv_draw_sdl_rect.o ./Middlewares/LVGL/GUI/lvgl/src/draw/sdl/lv_draw_sdl_rect.su ./Middlewares/LVGL/GUI/lvgl/src/draw/sdl/lv_draw_sdl_stack_blur.cyclo ./Middlewares/LVGL/GUI/lvgl/src/draw/sdl/lv_draw_sdl_stack_blur.d ./Middlewares/LVGL/GUI/lvgl/src/draw/sdl/lv_draw_sdl_stack_blur.o ./Middlewares/LVGL/GUI/lvgl/src/draw/sdl/lv_draw_sdl_stack_blur.su ./Middlewares/LVGL/GUI/lvgl/src/draw/sdl/lv_draw_sdl_texture_cache.cyclo ./Middlewares/LVGL/GUI/lvgl/src/draw/sdl/lv_draw_sdl_texture_cache.d ./Middlewares/LVGL/GUI/lvgl/src/draw/sdl/lv_draw_sdl_texture_cache.o ./Middlewares/LVGL/GUI/lvgl/src/draw/sdl/lv_draw_sdl_texture_cache.su ./Middlewares/LVGL/GUI/lvgl/src/draw/sdl/lv_draw_sdl_utils.cyclo ./Middlewares/LVGL/GUI/lvgl/src/draw/sdl/lv_draw_sdl_utils.d ./Middlewares/LVGL/GUI/lvgl/src/draw/sdl/lv_draw_sdl_utils.o ./Middlewares/LVGL/GUI/lvgl/src/draw/sdl/lv_draw_sdl_utils.su

.PHONY: clean-Middlewares-2f-LVGL-2f-GUI-2f-lvgl-2f-src-2f-draw-2f-sdl

