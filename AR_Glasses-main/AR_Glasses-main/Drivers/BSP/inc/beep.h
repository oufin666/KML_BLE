#ifndef BEEP_H
#define BEEP_H

#include "main.h"

void BEEP_Init(void);
void BEEP_SetVolume(uint8_t volume);
void BEEP_On(void);
void BEEP_Off(void);

#endif /* BEEP_H */