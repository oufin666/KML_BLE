################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/BSP/GH3220/Algo_code/goodix_algo_call/src/gh3x2x_demo_algo_call_adt.c \
../Drivers/BSP/GH3220/Algo_code/goodix_algo_call/src/gh3x2x_demo_algo_call_af.c \
../Drivers/BSP/GH3220/Algo_code/goodix_algo_call/src/gh3x2x_demo_algo_call_bt.c \
../Drivers/BSP/GH3220/Algo_code/goodix_algo_call/src/gh3x2x_demo_algo_call_ecg.c \
../Drivers/BSP/GH3220/Algo_code/goodix_algo_call/src/gh3x2x_demo_algo_call_hr.c \
../Drivers/BSP/GH3220/Algo_code/goodix_algo_call/src/gh3x2x_demo_algo_call_hrv.c \
../Drivers/BSP/GH3220/Algo_code/goodix_algo_call/src/gh3x2x_demo_algo_call_hsm.c \
../Drivers/BSP/GH3220/Algo_code/goodix_algo_call/src/gh3x2x_demo_algo_call_nadt.c \
../Drivers/BSP/GH3220/Algo_code/goodix_algo_call/src/gh3x2x_demo_algo_call_resp.c \
../Drivers/BSP/GH3220/Algo_code/goodix_algo_call/src/gh3x2x_demo_algo_call_spo2.c 

C_DEPS += \
./Drivers/BSP/GH3220/Algo_code/goodix_algo_call/src/gh3x2x_demo_algo_call_adt.d \
./Drivers/BSP/GH3220/Algo_code/goodix_algo_call/src/gh3x2x_demo_algo_call_af.d \
./Drivers/BSP/GH3220/Algo_code/goodix_algo_call/src/gh3x2x_demo_algo_call_bt.d \
./Drivers/BSP/GH3220/Algo_code/goodix_algo_call/src/gh3x2x_demo_algo_call_ecg.d \
./Drivers/BSP/GH3220/Algo_code/goodix_algo_call/src/gh3x2x_demo_algo_call_hr.d \
./Drivers/BSP/GH3220/Algo_code/goodix_algo_call/src/gh3x2x_demo_algo_call_hrv.d \
./Drivers/BSP/GH3220/Algo_code/goodix_algo_call/src/gh3x2x_demo_algo_call_hsm.d \
./Drivers/BSP/GH3220/Algo_code/goodix_algo_call/src/gh3x2x_demo_algo_call_nadt.d \
./Drivers/BSP/GH3220/Algo_code/goodix_algo_call/src/gh3x2x_demo_algo_call_resp.d \
./Drivers/BSP/GH3220/Algo_code/goodix_algo_call/src/gh3x2x_demo_algo_call_spo2.d 

OBJS += \
./Drivers/BSP/GH3220/Algo_code/goodix_algo_call/src/gh3x2x_demo_algo_call_adt.o \
./Drivers/BSP/GH3220/Algo_code/goodix_algo_call/src/gh3x2x_demo_algo_call_af.o \
./Drivers/BSP/GH3220/Algo_code/goodix_algo_call/src/gh3x2x_demo_algo_call_bt.o \
./Drivers/BSP/GH3220/Algo_code/goodix_algo_call/src/gh3x2x_demo_algo_call_ecg.o \
./Drivers/BSP/GH3220/Algo_code/goodix_algo_call/src/gh3x2x_demo_algo_call_hr.o \
./Drivers/BSP/GH3220/Algo_code/goodix_algo_call/src/gh3x2x_demo_algo_call_hrv.o \
./Drivers/BSP/GH3220/Algo_code/goodix_algo_call/src/gh3x2x_demo_algo_call_hsm.o \
./Drivers/BSP/GH3220/Algo_code/goodix_algo_call/src/gh3x2x_demo_algo_call_nadt.o \
./Drivers/BSP/GH3220/Algo_code/goodix_algo_call/src/gh3x2x_demo_algo_call_resp.o \
./Drivers/BSP/GH3220/Algo_code/goodix_algo_call/src/gh3x2x_demo_algo_call_spo2.o 


# Each subdirectory must supply rules for building sources it contributes
Drivers/BSP/GH3220/Algo_code/goodix_algo_call/src/%.o Drivers/BSP/GH3220/Algo_code/goodix_algo_call/src/%.su Drivers/BSP/GH3220/Algo_code/goodix_algo_call/src/%.cyclo: ../Drivers/BSP/GH3220/Algo_code/goodix_algo_call/src/%.c Drivers/BSP/GH3220/Algo_code/goodix_algo_call/src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F412Cx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../Drivers/BSP/inc -I../Drivers/BSP/src -I../Drivers/BSP/inc/kml_utils.h -I../Middlewares/LVGL/GUI -I../Middlewares/LVGL/GUI/lvgl -I../Middlewares/LVGL/GUI/src -I../Middlewares/LVGL/GUI/lvgl/examples/porting -I../FATFS/Target -I../FATFS/App -I../Middlewares/Third_Party/FatFs/src -I../Drivers/BSP/GH3220/Kernel_code/driver/inc -I../Drivers/BSP/GH3220/Kernel_code/kernel/inc -I../Drivers/BSP/GH3220/Kernel_code/module/gh_agc -I../Drivers/BSP/GH3220/Kernel_code/module/gh_ecg -I../Drivers/BSP/GH3220/Kernel_code/module/gh_other -I../Drivers/BSP/GH3220/Kernel_code/module/gh_protocol -I../Drivers/BSP/GH3220/Kernel_code/module/gh_soft_adt -I../Drivers/BSP/GH3220/Algo_code/goodix_algo_application/inc -I../Drivers/BSP/GH3220/Algo_code/goodix_algo_call/inc -I../Drivers/BSP/GH3220/Algo_code/goodix_algo_call/inc/hr_exc -I../Drivers/BSP/GH3220/Algo_code/goodix_algo_call/inc/spo2_pre_exc -I../Core/USER/APP/inc -I../Core/USER/APP/Pages/inc -I../Core/USER/Utils/lv_ext/inc -I../Core/USER/APP/Pages/Startup/inc -I../Core/USER/APP/Pages/Template/inc -I../Core/USER/APP/Pages/HeartRate/inc -I../Core/USER/Utils/Track_Parse/inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-BSP-2f-GH3220-2f-Algo_code-2f-goodix_algo_call-2f-src

clean-Drivers-2f-BSP-2f-GH3220-2f-Algo_code-2f-goodix_algo_call-2f-src:
	-$(RM) ./Drivers/BSP/GH3220/Algo_code/goodix_algo_call/src/gh3x2x_demo_algo_call_adt.cyclo ./Drivers/BSP/GH3220/Algo_code/goodix_algo_call/src/gh3x2x_demo_algo_call_adt.d ./Drivers/BSP/GH3220/Algo_code/goodix_algo_call/src/gh3x2x_demo_algo_call_adt.o ./Drivers/BSP/GH3220/Algo_code/goodix_algo_call/src/gh3x2x_demo_algo_call_adt.su ./Drivers/BSP/GH3220/Algo_code/goodix_algo_call/src/gh3x2x_demo_algo_call_af.cyclo ./Drivers/BSP/GH3220/Algo_code/goodix_algo_call/src/gh3x2x_demo_algo_call_af.d ./Drivers/BSP/GH3220/Algo_code/goodix_algo_call/src/gh3x2x_demo_algo_call_af.o ./Drivers/BSP/GH3220/Algo_code/goodix_algo_call/src/gh3x2x_demo_algo_call_af.su ./Drivers/BSP/GH3220/Algo_code/goodix_algo_call/src/gh3x2x_demo_algo_call_bt.cyclo ./Drivers/BSP/GH3220/Algo_code/goodix_algo_call/src/gh3x2x_demo_algo_call_bt.d ./Drivers/BSP/GH3220/Algo_code/goodix_algo_call/src/gh3x2x_demo_algo_call_bt.o ./Drivers/BSP/GH3220/Algo_code/goodix_algo_call/src/gh3x2x_demo_algo_call_bt.su ./Drivers/BSP/GH3220/Algo_code/goodix_algo_call/src/gh3x2x_demo_algo_call_ecg.cyclo ./Drivers/BSP/GH3220/Algo_code/goodix_algo_call/src/gh3x2x_demo_algo_call_ecg.d ./Drivers/BSP/GH3220/Algo_code/goodix_algo_call/src/gh3x2x_demo_algo_call_ecg.o ./Drivers/BSP/GH3220/Algo_code/goodix_algo_call/src/gh3x2x_demo_algo_call_ecg.su ./Drivers/BSP/GH3220/Algo_code/goodix_algo_call/src/gh3x2x_demo_algo_call_hr.cyclo ./Drivers/BSP/GH3220/Algo_code/goodix_algo_call/src/gh3x2x_demo_algo_call_hr.d ./Drivers/BSP/GH3220/Algo_code/goodix_algo_call/src/gh3x2x_demo_algo_call_hr.o ./Drivers/BSP/GH3220/Algo_code/goodix_algo_call/src/gh3x2x_demo_algo_call_hr.su ./Drivers/BSP/GH3220/Algo_code/goodix_algo_call/src/gh3x2x_demo_algo_call_hrv.cyclo ./Drivers/BSP/GH3220/Algo_code/goodix_algo_call/src/gh3x2x_demo_algo_call_hrv.d ./Drivers/BSP/GH3220/Algo_code/goodix_algo_call/src/gh3x2x_demo_algo_call_hrv.o ./Drivers/BSP/GH3220/Algo_code/goodix_algo_call/src/gh3x2x_demo_algo_call_hrv.su ./Drivers/BSP/GH3220/Algo_code/goodix_algo_call/src/gh3x2x_demo_algo_call_hsm.cyclo ./Drivers/BSP/GH3220/Algo_code/goodix_algo_call/src/gh3x2x_demo_algo_call_hsm.d ./Drivers/BSP/GH3220/Algo_code/goodix_algo_call/src/gh3x2x_demo_algo_call_hsm.o ./Drivers/BSP/GH3220/Algo_code/goodix_algo_call/src/gh3x2x_demo_algo_call_hsm.su ./Drivers/BSP/GH3220/Algo_code/goodix_algo_call/src/gh3x2x_demo_algo_call_nadt.cyclo ./Drivers/BSP/GH3220/Algo_code/goodix_algo_call/src/gh3x2x_demo_algo_call_nadt.d ./Drivers/BSP/GH3220/Algo_code/goodix_algo_call/src/gh3x2x_demo_algo_call_nadt.o ./Drivers/BSP/GH3220/Algo_code/goodix_algo_call/src/gh3x2x_demo_algo_call_nadt.su ./Drivers/BSP/GH3220/Algo_code/goodix_algo_call/src/gh3x2x_demo_algo_call_resp.cyclo ./Drivers/BSP/GH3220/Algo_code/goodix_algo_call/src/gh3x2x_demo_algo_call_resp.d ./Drivers/BSP/GH3220/Algo_code/goodix_algo_call/src/gh3x2x_demo_algo_call_resp.o ./Drivers/BSP/GH3220/Algo_code/goodix_algo_call/src/gh3x2x_demo_algo_call_resp.su ./Drivers/BSP/GH3220/Algo_code/goodix_algo_call/src/gh3x2x_demo_algo_call_spo2.cyclo ./Drivers/BSP/GH3220/Algo_code/goodix_algo_call/src/gh3x2x_demo_algo_call_spo2.d ./Drivers/BSP/GH3220/Algo_code/goodix_algo_call/src/gh3x2x_demo_algo_call_spo2.o ./Drivers/BSP/GH3220/Algo_code/goodix_algo_call/src/gh3x2x_demo_algo_call_spo2.su

.PHONY: clean-Drivers-2f-BSP-2f-GH3220-2f-Algo_code-2f-goodix_algo_call-2f-src

