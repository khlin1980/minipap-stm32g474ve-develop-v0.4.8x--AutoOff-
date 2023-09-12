#ifndef __IXAUTO_LEAKMANAGEMENT_H
#define __IXAUTO_LEAKMANAGEMENT_H
#include <stdint.h>
//void LeakManagementInit(unsigned int ZeroLeakBaseline[3]);
void LeakManagementInit(uint16_t ZeroLeakBaseline[3]);
void LeakManagementTask(float Flow, float TargetP, float FinalP, uint8_t withHumidifier, uint16_t* LeakQ, float* LeakP);

#endif