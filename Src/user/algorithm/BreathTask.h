/* iXAUTO_BreathTask.h*/
#ifndef __BreathTask_H
#define __BreathTask_H

#include "parameter.h"

//#define NoBreathAmplitude 1 // 20

//void BreathTaskProcess(struct Systems *system);
void BreathTaskProcess(float Pressure, float flow);
void BreathTaskInit(void);
uint8_t AutoOnDetect(uint16_t DP);

// void CDC_Transfer(struct Systems *system, uint8_t enableAutoOn, const short rawFlow);

#endif
