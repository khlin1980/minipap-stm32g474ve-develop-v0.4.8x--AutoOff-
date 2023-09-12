#ifndef __CPAP_H
#define __CPAP_H

#include <stdint.h>

void CPAPSetConfig(uint8_t Mode, uint8_t RampTime, uint8_t RampStartP, uint8_t PVALevel, uint8_t TreatmentPressure, uint8_t MinPressure, uint8_t MaxPressure);
void CPAPInit();
void CPAPProcess(int16_t Pressure, float Flow);
uint8_t CPAPAutoOn(uint16_t Flow);
uint8_t CPAPAutoOff();

#endif