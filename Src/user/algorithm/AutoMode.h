#ifndef __iXAUTO_AutoMode_H
#define __iXAUTO_AutoMode_H

//#define HI_AscendMaxInterval 600
//#define AI_AscendBreathTime 400
//#define BreathStatus_Apnea 2


//void AutoMode(uint16_t Breath_Amp, uint8_t BreathStatus,uint16_t NormalTHRE,uint16_t ApneaTHRE,uint16_t HypopneaTHRE);

//extern int A;
//extern int F;
//extern int SampleRate_Count;
//extern unsigned int state_timeout;
//extern uint8_t AUTO_S;

void AutoModeTaskInit(float minP, float maxP, int SamplePerSecond);
void AutoModeTaskProcess(uint8_t BreathStatus, float* TargetPressure, float* RecordTargetPressure);
//void AutoModeTaskProcess(uint8_t BreathStatus, float* TargetPressure);
//extern float max_pressure_setting;
//extern float min_pressure_setting;
//extern uint8_t Ascend01End;
#endif