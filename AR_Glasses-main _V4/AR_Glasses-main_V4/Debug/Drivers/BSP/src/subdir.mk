################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/BSP/src/BLE.c \
../Drivers/BSP/src/Encoder.c \
../Drivers/BSP/src/GH3220.c \
../Drivers/BSP/src/GPS.c \
../Drivers/BSP/src/LCD.c \
../Drivers/BSP/src/LCD_TK024F3036.c \
../Drivers/BSP/src/SD_opera.c \
../Drivers/BSP/src/my_spi.c 

C_DEPS += \
./Drivers/BSP/src/BLE.d \
./Drivers/BSP/src/Encoder.d \
./Drivers/BSP/src/GH3220.d \
./Drivers/BSP/src/GPS.d \
./Drivers/BSP/src/LCD.d \
./Drivers/BSP/src/LCD_TK024F3036.d \
./Drivers/BSP/src/SD_opera.d \
./Drivers/BSP/src/my_spi.d 

OBJS += \
./Drivers/BSP/src/BLE.o \
./Drivers/BSP/src/Encoder.o \
./Drivers/BSP/src/GH3220.o \
./Drivers/BSP/src/GPS.o \
./Drivers/BSP/src/LCD.o \
./Drivers/BSP/src/LCD_TK024F3036.o \
./Drivers/BSP/src/SD_opera.o \
./Drivers/BSP/src/my_spi.o 


# Each subdirectory must supply rules for building sources it contributes
Drivers/BSP/src/%.o Drivers/BSP/src/%.su Drivers/BSP/src/%.cyclo: ../Drivers/BSP/src/%.c Drivers/BSP/src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F412Cx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../Drivers/BSP/inc -I../Middlewares/LVGL/GUI -I../Middlewares/LVGL/GUI/lvgl -I../Middlewares/LVGL/GUI/src -I../Middlewares/LVGL/GUI/lvgl/examples/porting -I../FATFS/Target -I../FATFS/App -I../Middlewares/Third_Party/FatFs/src -I../Drivers/BSP/GH3220/Kernel_code/driver/inc -I../Drivers/BSP/GH3220/Kernel_code/kernel/inc -I../Drivers/BSP/GH3220/Kernel_code/module/gh_agc -I../Drivers/BSP/GH3220/Kernel_code/module/gh_ecg -I../Drivers/BSP/GH3220/Kernel_code/module/gh_other -I../Drivers/BSP/GH3220/Kernel_code/module/gh_protocol -I../Drivers/BSP/GH3220/Kernel_code/module/gh_soft_adt -I../Drivers/BSP/GH3220/Algo_code/goodix_algo_application/inc -I../Drivers/BSP/GH3220/Algo_code/goodix_algo_call/inc -I../Drivers/BSP/GH3220/Algo_code/goodix_algo_call/inc/hr_exc -I../Drivers/BSP/GH3220/Algo_code/goodix_algo_call/inc/spo2_pre_exc -I../Core/USER/APP/inc -I../Core/USER/APP/Pages/inc -I../Core/USER/Utils/lv_ext/inc -I../Core/USER/APP/Pages/Startup/inc -I../Core/USER/APP/Pages/Template/inc -I../Core/USER/APP/Pages/HeartRate/inc -I../Core/USER/Utils/Track_Parse/inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-BSP-2f-src

clean-Drivers-2f-BSP-2f-src:
	-$(RM) ./Drivers/BSP/src/BLE.cyclo ./Drivers/BSP/src/BLE.d ./Drivers/BSP/src/BLE.o ./Drivers/BSP/src/BLE.su ./Drivers/BSP/src/Encoder.cyclo ./Drivers/BSP/src/Encoder.d ./Drivers/BSP/src/Encoder.o ./Drivers/BSP/src/Encoder.su ./Drivers/BSP/src/GH3220.cyclo ./Drivers/BSP/src/GH3220.d ./Drivers/BSP/src/GH3220.o ./Drivers/BSP/src/GH3220.su ./Drivers/BSP/src/GPS.cyclo ./Drivers/BSP/src/GPS.d ./Drivers/BSP/src/GPS.o ./Drivers/BSP/src/GPS.su ./Drivers/BSP/src/LCD.cyclo ./Drivers/BSP/src/LCD.d ./Drivers/BSP/src/LCD.o ./Drivers/BSP/src/LCD.su ./Drivers/BSP/src/LCD_TK024F3036.cyclo ./Drivers/BSP/src/LCD_TK024F3036.d ./Drivers/BSP/src/LCD_TK024F3036.o ./Drivers/BSP/src/LCD_TK024F3036.su ./Drivers/BSP/src/SD_opera.cyclo ./Drivers/BSP/src/SD_opera.d ./Drivers/BSP/src/SD_opera.o ./Drivers/BSP/src/SD_opera.su ./Drivers/BSP/src/my_spi.cyclo ./Drivers/BSP/src/my_spi.d ./Drivers/BSP/src/my_spi.o ./Drivers/BSP/src/my_spi.su

.PHONY: clean-Drivers-2f-BSP-2f-src

