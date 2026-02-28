################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Middlewares/LVGL/GUI/lvgl/src/widgets/lv_arc.c \
../Middlewares/LVGL/GUI/lvgl/src/widgets/lv_bar.c \
../Middlewares/LVGL/GUI/lvgl/src/widgets/lv_btn.c \
../Middlewares/LVGL/GUI/lvgl/src/widgets/lv_btnmatrix.c \
../Middlewares/LVGL/GUI/lvgl/src/widgets/lv_canvas.c \
../Middlewares/LVGL/GUI/lvgl/src/widgets/lv_checkbox.c \
../Middlewares/LVGL/GUI/lvgl/src/widgets/lv_dropdown.c \
../Middlewares/LVGL/GUI/lvgl/src/widgets/lv_img.c \
../Middlewares/LVGL/GUI/lvgl/src/widgets/lv_label.c \
../Middlewares/LVGL/GUI/lvgl/src/widgets/lv_line.c \
../Middlewares/LVGL/GUI/lvgl/src/widgets/lv_objx_templ.c \
../Middlewares/LVGL/GUI/lvgl/src/widgets/lv_roller.c \
../Middlewares/LVGL/GUI/lvgl/src/widgets/lv_slider.c \
../Middlewares/LVGL/GUI/lvgl/src/widgets/lv_switch.c \
../Middlewares/LVGL/GUI/lvgl/src/widgets/lv_table.c \
../Middlewares/LVGL/GUI/lvgl/src/widgets/lv_textarea.c 

C_DEPS += \
./Middlewares/LVGL/GUI/lvgl/src/widgets/lv_arc.d \
./Middlewares/LVGL/GUI/lvgl/src/widgets/lv_bar.d \
./Middlewares/LVGL/GUI/lvgl/src/widgets/lv_btn.d \
./Middlewares/LVGL/GUI/lvgl/src/widgets/lv_btnmatrix.d \
./Middlewares/LVGL/GUI/lvgl/src/widgets/lv_canvas.d \
./Middlewares/LVGL/GUI/lvgl/src/widgets/lv_checkbox.d \
./Middlewares/LVGL/GUI/lvgl/src/widgets/lv_dropdown.d \
./Middlewares/LVGL/GUI/lvgl/src/widgets/lv_img.d \
./Middlewares/LVGL/GUI/lvgl/src/widgets/lv_label.d \
./Middlewares/LVGL/GUI/lvgl/src/widgets/lv_line.d \
./Middlewares/LVGL/GUI/lvgl/src/widgets/lv_objx_templ.d \
./Middlewares/LVGL/GUI/lvgl/src/widgets/lv_roller.d \
./Middlewares/LVGL/GUI/lvgl/src/widgets/lv_slider.d \
./Middlewares/LVGL/GUI/lvgl/src/widgets/lv_switch.d \
./Middlewares/LVGL/GUI/lvgl/src/widgets/lv_table.d \
./Middlewares/LVGL/GUI/lvgl/src/widgets/lv_textarea.d 

OBJS += \
./Middlewares/LVGL/GUI/lvgl/src/widgets/lv_arc.o \
./Middlewares/LVGL/GUI/lvgl/src/widgets/lv_bar.o \
./Middlewares/LVGL/GUI/lvgl/src/widgets/lv_btn.o \
./Middlewares/LVGL/GUI/lvgl/src/widgets/lv_btnmatrix.o \
./Middlewares/LVGL/GUI/lvgl/src/widgets/lv_canvas.o \
./Middlewares/LVGL/GUI/lvgl/src/widgets/lv_checkbox.o \
./Middlewares/LVGL/GUI/lvgl/src/widgets/lv_dropdown.o \
./Middlewares/LVGL/GUI/lvgl/src/widgets/lv_img.o \
./Middlewares/LVGL/GUI/lvgl/src/widgets/lv_label.o \
./Middlewares/LVGL/GUI/lvgl/src/widgets/lv_line.o \
./Middlewares/LVGL/GUI/lvgl/src/widgets/lv_objx_templ.o \
./Middlewares/LVGL/GUI/lvgl/src/widgets/lv_roller.o \
./Middlewares/LVGL/GUI/lvgl/src/widgets/lv_slider.o \
./Middlewares/LVGL/GUI/lvgl/src/widgets/lv_switch.o \
./Middlewares/LVGL/GUI/lvgl/src/widgets/lv_table.o \
./Middlewares/LVGL/GUI/lvgl/src/widgets/lv_textarea.o 


# Each subdirectory must supply rules for building sources it contributes
Middlewares/LVGL/GUI/lvgl/src/widgets/%.o Middlewares/LVGL/GUI/lvgl/src/widgets/%.su Middlewares/LVGL/GUI/lvgl/src/widgets/%.cyclo: ../Middlewares/LVGL/GUI/lvgl/src/widgets/%.c Middlewares/LVGL/GUI/lvgl/src/widgets/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F412Cx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../Drivers/BSP/inc -I../Drivers/BSP/src -I../Drivers/BSP/inc/kml_utils.h -I../Middlewares/LVGL/GUI -I../Middlewares/LVGL/GUI/lvgl -I../Middlewares/LVGL/GUI/src -I../Middlewares/LVGL/GUI/lvgl/examples/porting -I../FATFS/Target -I../FATFS/App -I../Middlewares/Third_Party/FatFs/src -I../Drivers/BSP/GH3220/Kernel_code/driver/inc -I../Drivers/BSP/GH3220/Kernel_code/kernel/inc -I../Drivers/BSP/GH3220/Kernel_code/module/gh_agc -I../Drivers/BSP/GH3220/Kernel_code/module/gh_ecg -I../Drivers/BSP/GH3220/Kernel_code/module/gh_other -I../Drivers/BSP/GH3220/Kernel_code/module/gh_protocol -I../Drivers/BSP/GH3220/Kernel_code/module/gh_soft_adt -I../Drivers/BSP/GH3220/Algo_code/goodix_algo_application/inc -I../Drivers/BSP/GH3220/Algo_code/goodix_algo_call/inc -I../Drivers/BSP/GH3220/Algo_code/goodix_algo_call/inc/hr_exc -I../Drivers/BSP/GH3220/Algo_code/goodix_algo_call/inc/spo2_pre_exc -I../Core/USER/APP/inc -I../Core/USER/APP/Pages/inc -I../Core/USER/Utils/lv_ext/inc -I../Core/USER/APP/Pages/Startup/inc -I../Core/USER/APP/Pages/Template/inc -I../Core/USER/APP/Pages/HeartRate/inc -I../Core/USER/Utils/Track_Parse/inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Middlewares-2f-LVGL-2f-GUI-2f-lvgl-2f-src-2f-widgets

clean-Middlewares-2f-LVGL-2f-GUI-2f-lvgl-2f-src-2f-widgets:
	-$(RM) ./Middlewares/LVGL/GUI/lvgl/src/widgets/lv_arc.cyclo ./Middlewares/LVGL/GUI/lvgl/src/widgets/lv_arc.d ./Middlewares/LVGL/GUI/lvgl/src/widgets/lv_arc.o ./Middlewares/LVGL/GUI/lvgl/src/widgets/lv_arc.su ./Middlewares/LVGL/GUI/lvgl/src/widgets/lv_bar.cyclo ./Middlewares/LVGL/GUI/lvgl/src/widgets/lv_bar.d ./Middlewares/LVGL/GUI/lvgl/src/widgets/lv_bar.o ./Middlewares/LVGL/GUI/lvgl/src/widgets/lv_bar.su ./Middlewares/LVGL/GUI/lvgl/src/widgets/lv_btn.cyclo ./Middlewares/LVGL/GUI/lvgl/src/widgets/lv_btn.d ./Middlewares/LVGL/GUI/lvgl/src/widgets/lv_btn.o ./Middlewares/LVGL/GUI/lvgl/src/widgets/lv_btn.su ./Middlewares/LVGL/GUI/lvgl/src/widgets/lv_btnmatrix.cyclo ./Middlewares/LVGL/GUI/lvgl/src/widgets/lv_btnmatrix.d ./Middlewares/LVGL/GUI/lvgl/src/widgets/lv_btnmatrix.o ./Middlewares/LVGL/GUI/lvgl/src/widgets/lv_btnmatrix.su ./Middlewares/LVGL/GUI/lvgl/src/widgets/lv_canvas.cyclo ./Middlewares/LVGL/GUI/lvgl/src/widgets/lv_canvas.d ./Middlewares/LVGL/GUI/lvgl/src/widgets/lv_canvas.o ./Middlewares/LVGL/GUI/lvgl/src/widgets/lv_canvas.su ./Middlewares/LVGL/GUI/lvgl/src/widgets/lv_checkbox.cyclo ./Middlewares/LVGL/GUI/lvgl/src/widgets/lv_checkbox.d ./Middlewares/LVGL/GUI/lvgl/src/widgets/lv_checkbox.o ./Middlewares/LVGL/GUI/lvgl/src/widgets/lv_checkbox.su ./Middlewares/LVGL/GUI/lvgl/src/widgets/lv_dropdown.cyclo ./Middlewares/LVGL/GUI/lvgl/src/widgets/lv_dropdown.d ./Middlewares/LVGL/GUI/lvgl/src/widgets/lv_dropdown.o ./Middlewares/LVGL/GUI/lvgl/src/widgets/lv_dropdown.su ./Middlewares/LVGL/GUI/lvgl/src/widgets/lv_img.cyclo ./Middlewares/LVGL/GUI/lvgl/src/widgets/lv_img.d ./Middlewares/LVGL/GUI/lvgl/src/widgets/lv_img.o ./Middlewares/LVGL/GUI/lvgl/src/widgets/lv_img.su ./Middlewares/LVGL/GUI/lvgl/src/widgets/lv_label.cyclo ./Middlewares/LVGL/GUI/lvgl/src/widgets/lv_label.d ./Middlewares/LVGL/GUI/lvgl/src/widgets/lv_label.o ./Middlewares/LVGL/GUI/lvgl/src/widgets/lv_label.su ./Middlewares/LVGL/GUI/lvgl/src/widgets/lv_line.cyclo ./Middlewares/LVGL/GUI/lvgl/src/widgets/lv_line.d ./Middlewares/LVGL/GUI/lvgl/src/widgets/lv_line.o ./Middlewares/LVGL/GUI/lvgl/src/widgets/lv_line.su ./Middlewares/LVGL/GUI/lvgl/src/widgets/lv_objx_templ.cyclo ./Middlewares/LVGL/GUI/lvgl/src/widgets/lv_objx_templ.d ./Middlewares/LVGL/GUI/lvgl/src/widgets/lv_objx_templ.o ./Middlewares/LVGL/GUI/lvgl/src/widgets/lv_objx_templ.su ./Middlewares/LVGL/GUI/lvgl/src/widgets/lv_roller.cyclo ./Middlewares/LVGL/GUI/lvgl/src/widgets/lv_roller.d ./Middlewares/LVGL/GUI/lvgl/src/widgets/lv_roller.o ./Middlewares/LVGL/GUI/lvgl/src/widgets/lv_roller.su ./Middlewares/LVGL/GUI/lvgl/src/widgets/lv_slider.cyclo ./Middlewares/LVGL/GUI/lvgl/src/widgets/lv_slider.d ./Middlewares/LVGL/GUI/lvgl/src/widgets/lv_slider.o ./Middlewares/LVGL/GUI/lvgl/src/widgets/lv_slider.su ./Middlewares/LVGL/GUI/lvgl/src/widgets/lv_switch.cyclo ./Middlewares/LVGL/GUI/lvgl/src/widgets/lv_switch.d ./Middlewares/LVGL/GUI/lvgl/src/widgets/lv_switch.o ./Middlewares/LVGL/GUI/lvgl/src/widgets/lv_switch.su ./Middlewares/LVGL/GUI/lvgl/src/widgets/lv_table.cyclo ./Middlewares/LVGL/GUI/lvgl/src/widgets/lv_table.d ./Middlewares/LVGL/GUI/lvgl/src/widgets/lv_table.o ./Middlewares/LVGL/GUI/lvgl/src/widgets/lv_table.su ./Middlewares/LVGL/GUI/lvgl/src/widgets/lv_textarea.cyclo ./Middlewares/LVGL/GUI/lvgl/src/widgets/lv_textarea.d ./Middlewares/LVGL/GUI/lvgl/src/widgets/lv_textarea.o ./Middlewares/LVGL/GUI/lvgl/src/widgets/lv_textarea.su

.PHONY: clean-Middlewares-2f-LVGL-2f-GUI-2f-lvgl-2f-src-2f-widgets

