#ifndef BEEP_H
#define BEEP_H

#include "main.h"

extern uint8_t beep_enabled; // 声明为外部变量

void BEEP_Init(void);
void BEEP_Update(void);
void BEEP_On(void);
void BEEP_Off(void);

#endif /* BEEP_H */