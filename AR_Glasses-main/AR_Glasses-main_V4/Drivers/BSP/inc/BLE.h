#ifndef BLE_H
#define BLE_H

#include "main.h"

#ifdef __cplusplus
extern "C" {
#endif

void Bluetooth_Init(void);
uint8_t Get_Heart_Rate(void);
void BLE_UART_RxCpltCallback(UART_HandleTypeDef *huart);

#ifdef __cplusplus
}
#endif

#endif
