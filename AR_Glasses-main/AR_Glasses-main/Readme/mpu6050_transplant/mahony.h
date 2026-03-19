#ifndef MAHONY_H
#define MAHONY_H

#include "zf_common_headfile.h"
#include <math.h>
#include "configue.h"

extern float q0, q1, q2, q3;

void Mahony_Init(float sampleFrequency);
void MahonyAHRSupdateIMU(float gx, float gy, float gz, float ax, float ay, float az);
void Mahony_computeAngles();
float getYaw();

#endif
