#ifndef MOTOR_H
#define MOTOR_H

#include "main.h"

void MOTOR_Init(void);
void MOTOR_SetSpeed(uint8_t speed);
void MOTOR_On(void);
void MOTOR_Off(void);

#endif /* MOTOR_H */