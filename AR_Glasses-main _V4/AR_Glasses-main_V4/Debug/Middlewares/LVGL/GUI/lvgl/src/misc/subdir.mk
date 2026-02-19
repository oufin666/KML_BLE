################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Middlewares/LVGL/GUI/lvgl/src/misc/lv_anim.c \
../Middlewares/LVGL/GUI/lvgl/src/misc/lv_anim_timeline.c \
../Middlewares/LVGL/GUI/lvgl/src/misc/lv_area.c \
../Middlewares/LVGL/GUI/lvgl/src/misc/lv_async.c \
../Middlewares/LVGL/GUI/lvgl/src/misc/lv_bidi.c \
../Middlewares/LVGL/GUI/lvgl/src/misc/lv_color.c \
../Middlewares/LVGL/GUI/lvgl/src/misc/lv_fs.c \
../Middlewares/LVGL/GUI/lvgl/src/misc/lv_gc.c \
../Middlewares/LVGL/GUI/lvgl/src/misc/lv_ll.c \
../Middlewares/LVGL/GUI/lvgl/src/misc/lv_log.c \
../Middlewares/LVGL/GUI/lvgl/src/misc/lv_lru.c \
../Middlewares/LVGL/GUI/lvgl/src/misc/lv_math.c \
../Middlewares/LVGL/GUI/lvgl/src/misc/lv_mem.c \
../Middlewares/LVGL/GUI/lvgl/src/misc/lv_printf.c \
../Middlewares/LVGL/GUI/lvgl/src/misc/lv_style.c \
../Middlewares/LVGL/GUI/lvgl/src/misc/lv_style_gen.c \
../Middlewares/LVGL/GUI/lvgl/src/misc/lv_templ.c \
../Middlewares/LVGL/GUI/lvgl/src/misc/lv_timer.c \
../Middlewares/LVGL/GUI/lvgl/src/misc/lv_tlsf.c \
../Middlewares/LVGL/GUI/lvgl/src/misc/lv_txt.c \
../Middlewares/LVGL/GUI/lvgl/src/misc/lv_txt_ap.c \
../Middlewares/LVGL/GUI/lvgl/src/misc/lv_utils.c 

C_DEPS += \
./Middlewares/LVGL/GUI/lvgl/src/misc/lv_anim.d \
./Middlewares/LVGL/GUI/lvgl/src/misc/lv_anim_timeline.d \
./Middlewares/LVGL/GUI/lvgl/src/misc/lv_area.d \
./Middlewares/LVGL/GUI/lvgl/src/misc/lv_async.d \
./Middlewares/LVGL/GUI/lvgl/src/misc/lv_bidi.d \
./Middlewares/LVGL/GUI/lvgl/src/misc/lv_color.d \
./Middlewares/LVGL/GUI/lvgl/src/misc/lv_fs.d \
./Middlewares/LVGL/GUI/lvgl/src/misc/lv_gc.d \
./Middlewares/LVGL/GUI/lvgl/src/misc/lv_ll.d \
./Middlewares/LVGL/GUI/lvgl/src/misc/lv_log.d \
./Middlewares/LVGL/GUI/lvgl/src/misc/lv_lru.d \
./Middlewares/LVGL/GUI/lvgl/src/misc/lv_math.d \
./Middlewares/LVGL/GUI/lvgl/src/misc/lv_mem.d \
./Middlewares/LVGL/GUI/lvgl/src/misc/lv_printf.d \
./Middlewares/LVGL/GUI/lvgl/src/misc/lv_style.d \
./Middlewares/LVGL/GUI/lvgl/src/misc/lv_style_gen.d \
./Middlewares/LVGL/GUI/lvgl/src/misc/lv_templ.d \
./Middlewares/LVGL/GUI/lvgl/src/misc/lv_timer.d \
./Middlewares/LVGL/GUI/lvgl/src/misc/lv_tlsf.d \
./Middlewares/LVGL/GUI/lvgl/src/misc/lv_txt.d \
./Middlewares/LVGL/GUI/lvgl/src/misc/lv_txt_ap.d \
./Middlewares/LVGL/GUI/lvgl/src/misc/lv_utils.d 

OBJS += \
./Middlewares/LVGL/GUI/lvgl/src/misc/lv_anim.o \
./Middlewares/LVGL/GUI/lvgl/src/misc/lv_anim_timeline.o \
./Middlewares/LVGL/GUI/lvgl/src/misc/lv_area.o \
./Middlewares/LVGL/GUI/lvgl/src/misc/lv_async.o \
./Middlewares/LVGL/GUI/lvgl/src/misc/lv_bidi.o \
./Middlewares/LVGL/GUI/lvgl/src/misc/lv_color.o \
./Middlewares/LVGL/GUI/lvgl/src/misc/lv_fs.o \
./Middlewares/LVGL/GUI/lvgl/src/misc/lv_gc.o \
./Middlewares/LVGL/GUI/lvgl/src/misc/lv_ll.o \
./Middlewares/LVGL/GUI/lvgl/src/misc/lv_log.o \
./Middlewares/LVGL/GUI/lvgl/src/misc/lv_lru.o \
./Middlewares/LVGL/GUI/lvgl/src/misc/lv_math.o \
./Middlewares/LVGL/GUI/lvgl/src/misc/lv_mem.o \
./Middlewares/LVGL/GUI/lvgl/src/misc/lv_printf.o \
./Middlewares/LVGL/GUI/lvgl/src/misc/lv_style.o \
./Middlewares/LVGL/GUI/lvgl/src/misc/lv_style_gen.o \
./Middlewares/LVGL/GUI/lvgl/src/misc/lv_templ.o \
./Middlewares/LVGL/GUI/lvgl/src/misc/lv_timer.o \
./Middlewares/LVGL/GUI/lvgl/src/misc/lv_tlsf.o \
./Middlewares/LVGL/GUI/lvgl/src/misc/lv_txt.o \
./Middlewares/LVGL/GUI/lvgl/src/misc/lv_txt_ap.o \
./Middlewares/LVGL/GUI/lvgl/src/misc/lv_utils.o 


# Each subdirectory must supply rules for building sources it contributes
Middlewares/LVGL/GUI/lvgl/src/misc/%.o Middlewares/LVGL/GUI/lvgl/src/misc/%.su Middlewares/LVGL/GUI/lvgl/src/misc/%.cyclo: ../Middlewares/LVGL/GUI/lvgl/src/misc/%.c Middlewares/LVGL/GUI/lvgl/src/misc/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F412Cx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../Drivers/BSP/inc -I../Middlewares/LVGL/GUI -I../Middlewares/LVGL/GUI/lvgl -I../Middlewares/LVGL/GUI/src -I../Middlewares/LVGL/GUI/lvgl/examples/porting -I../FATFS/Target -I../FATFS/App -I../Middlewares/Third_Party/FatFs/src -I../Drivers/BSP/GH3220/Kernel_code/driver/inc -I../Drivers/BSP/GH3220/Kernel_code/kernel/inc -I../Drivers/BSP/GH3220/Kernel_code/module/gh_agc -I../Drivers/BSP/GH3220/Kernel_code/module/gh_ecg -I../Drivers/BSP/GH3220/Kernel_code/module/gh_other -I../Drivers/BSP/GH3220/Kernel_code/module/gh_protocol -I../Drivers/BSP/GH3220/Kernel_code/module/gh_soft_adt -I../Drivers/BSP/GH3220/Algo_code/goodix_algo_application/inc -I../Drivers/BSP/GH3220/Algo_code/goodix_algo_call/inc -I../Drivers/BSP/GH3220/Algo_code/goodix_algo_call/inc/hr_exc -I../Drivers/BSP/GH3220/Algo_code/goodix_algo_call/inc/spo2_pre_exc -I../Core/USER/APP/inc -I../Core/USER/APP/Pages/inc -I../Core/USER/Utils/lv_ext/inc -I../Core/USER/APP/Pages/Startup/inc -I../Core/USER/APP/Pages/Template/inc -I../Core/USER/APP/Pages/HeartRate/inc -I../Core/USER/Utils/Track_Parse/inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Middlewares-2f-LVGL-2f-GUI-2f-lvgl-2f-src-2f-misc

clean-Middlewares-2f-LVGL-2f-GUI-2f-lvgl-2f-src-2f-misc:
	-$(RM) ./Middlewares/LVGL/GUI/lvgl/src/misc/lv_anim.cyclo ./Middlewares/LVGL/GUI/lvgl/src/misc/lv_anim.d ./Middlewares/LVGL/GUI/lvgl/src/misc/lv_anim.o ./Middlewares/LVGL/GUI/lvgl/src/misc/lv_anim.su ./Middlewares/LVGL/GUI/lvgl/src/misc/lv_anim_timeline.cyclo ./Middlewares/LVGL/GUI/lvgl/src/misc/lv_anim_timeline.d ./Middlewares/LVGL/GUI/lvgl/src/misc/lv_anim_timeline.o ./Middlewares/LVGL/GUI/lvgl/src/misc/lv_anim_timeline.su ./Middlewares/LVGL/GUI/lvgl/src/misc/lv_area.cyclo ./Middlewares/LVGL/GUI/lvgl/src/misc/lv_area.d ./Middlewares/LVGL/GUI/lvgl/src/misc/lv_area.o ./Middlewares/LVGL/GUI/lvgl/src/misc/lv_area.su ./Middlewares/LVGL/GUI/lvgl/src/misc/lv_async.cyclo ./Middlewares/LVGL/GUI/lvgl/src/misc/lv_async.d ./Middlewares/LVGL/GUI/lvgl/src/misc/lv_async.o ./Middlewares/LVGL/GUI/lvgl/src/misc/lv_async.su ./Middlewares/LVGL/GUI/lvgl/src/misc/lv_bidi.cyclo ./Middlewares/LVGL/GUI/lvgl/src/misc/lv_bidi.d ./Middlewares/LVGL/GUI/lvgl/src/misc/lv_bidi.o ./Middlewares/LVGL/GUI/lvgl/src/misc/lv_bidi.su ./Middlewares/LVGL/GUI/lvgl/src/misc/lv_color.cyclo ./Middlewares/LVGL/GUI/lvgl/src/misc/lv_color.d ./Middlewares/LVGL/GUI/lvgl/src/misc/lv_color.o ./Middlewares/LVGL/GUI/lvgl/src/misc/lv_color.su ./Middlewares/LVGL/GUI/lvgl/src/misc/lv_fs.cyclo ./Middlewares/LVGL/GUI/lvgl/src/misc/lv_fs.d ./Middlewares/LVGL/GUI/lvgl/src/misc/lv_fs.o ./Middlewares/LVGL/GUI/lvgl/src/misc/lv_fs.su ./Middlewares/LVGL/GUI/lvgl/src/misc/lv_gc.cyclo ./Middlewares/LVGL/GUI/lvgl/src/misc/lv_gc.d ./Middlewares/LVGL/GUI/lvgl/src/misc/lv_gc.o ./Middlewares/LVGL/GUI/lvgl/src/misc/lv_gc.su ./Middlewares/LVGL/GUI/lvgl/src/misc/lv_ll.cyclo ./Middlewares/LVGL/GUI/lvgl/src/misc/lv_ll.d ./Middlewares/LVGL/GUI/lvgl/src/misc/lv_ll.o ./Middlewares/LVGL/GUI/lvgl/src/misc/lv_ll.su ./Middlewares/LVGL/GUI/lvgl/src/misc/lv_log.cyclo ./Middlewares/LVGL/GUI/lvgl/src/misc/lv_log.d ./Middlewares/LVGL/GUI/lvgl/src/misc/lv_log.o ./Middlewares/LVGL/GUI/lvgl/src/misc/lv_log.su ./Middlewares/LVGL/GUI/lvgl/src/misc/lv_lru.cyclo ./Middlewares/LVGL/GUI/lvgl/src/misc/lv_lru.d ./Middlewares/LVGL/GUI/lvgl/src/misc/lv_lru.o ./Middlewares/LVGL/GUI/lvgl/src/misc/lv_lru.su ./Middlewares/LVGL/GUI/lvgl/src/misc/lv_math.cyclo ./Middlewares/LVGL/GUI/lvgl/src/misc/lv_math.d ./Middlewares/LVGL/GUI/lvgl/src/misc/lv_math.o ./Middlewares/LVGL/GUI/lvgl/src/misc/lv_math.su ./Middlewares/LVGL/GUI/lvgl/src/misc/lv_mem.cyclo ./Middlewares/LVGL/GUI/lvgl/src/misc/lv_mem.d ./Middlewares/LVGL/GUI/lvgl/src/misc/lv_mem.o ./Middlewares/LVGL/GUI/lvgl/src/misc/lv_mem.su ./Middlewares/LVGL/GUI/lvgl/src/misc/lv_printf.cyclo ./Middlewares/LVGL/GUI/lvgl/src/misc/lv_printf.d ./Middlewares/LVGL/GUI/lvgl/src/misc/lv_printf.o ./Middlewares/LVGL/GUI/lvgl/src/misc/lv_printf.su ./Middlewares/LVGL/GUI/lvgl/src/misc/lv_style.cyclo ./Middlewares/LVGL/GUI/lvgl/src/misc/lv_style.d ./Middlewares/LVGL/GUI/lvgl/src/misc/lv_style.o ./Middlewares/LVGL/GUI/lvgl/src/misc/lv_style.su ./Middlewares/LVGL/GUI/lvgl/src/misc/lv_style_gen.cyclo ./Middlewares/LVGL/GUI/lvgl/src/misc/lv_style_gen.d ./Middlewares/LVGL/GUI/lvgl/src/misc/lv_style_gen.o ./Middlewares/LVGL/GUI/lvgl/src/misc/lv_style_gen.su ./Middlewares/LVGL/GUI/lvgl/src/misc/lv_templ.cyclo ./Middlewares/LVGL/GUI/lvgl/src/misc/lv_templ.d ./Middlewares/LVGL/GUI/lvgl/src/misc/lv_templ.o ./Middlewares/LVGL/GUI/lvgl/src/misc/lv_templ.su ./Middlewares/LVGL/GUI/lvgl/src/misc/lv_timer.cyclo ./Middlewares/LVGL/GUI/lvgl/src/misc/lv_timer.d ./Middlewares/LVGL/GUI/lvgl/src/misc/lv_timer.o ./Middlewares/LVGL/GUI/lvgl/src/misc/lv_timer.su ./Middlewares/LVGL/GUI/lvgl/src/misc/lv_tlsf.cyclo ./Middlewares/LVGL/GUI/lvgl/src/misc/lv_tlsf.d ./Middlewares/LVGL/GUI/lvgl/src/misc/lv_tlsf.o ./Middlewares/LVGL/GUI/lvgl/src/misc/lv_tlsf.su ./Middlewares/LVGL/GUI/lvgl/src/misc/lv_txt.cyclo ./Middlewares/LVGL/GUI/lvgl/src/misc/lv_txt.d ./Middlewares/LVGL/GUI/lvgl/src/misc/lv_txt.o ./Middlewares/LVGL/GUI/lvgl/src/misc/lv_txt.su ./Middlewares/LVGL/GUI/lvgl/src/misc/lv_txt_ap.cyclo ./Middlewares/LVGL/GUI/lvgl/src/misc/lv_txt_ap.d ./Middlewares/LVGL/GUI/lvgl/src/misc/lv_txt_ap.o ./Middlewares/LVGL/GUI/lvgl/src/misc/lv_txt_ap.su ./Middlewares/LVGL/GUI/lvgl/src/misc/lv_utils.cyclo ./Middlewares/LVGL/GUI/lvgl/src/misc/lv_utils.d ./Middlewares/LVGL/GUI/lvgl/src/misc/lv_utils.o ./Middlewares/LVGL/GUI/lvgl/src/misc/lv_utils.su

.PHONY: clean-Middlewares-2f-LVGL-2f-GUI-2f-lvgl-2f-src-2f-misc

