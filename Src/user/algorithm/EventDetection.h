#ifndef __AHIDetection_H
#define __AHIDetection_H

typedef struct
{
    uint16_t IndexOfOSA;
    uint16_t IndexOfCSA;
    uint16_t IndexOfHypopnea;
    uint16_t IndexOfFL; // Mei add
    uint8_t EventHappenedCount;
    uint8_t EventCount;
    uint8_t EventTypes[20];
    //uint8_t DescendFlag;

} TypeAHI_info;

// uint8_t BreathStatus;

// uint8_t EventDetectionTaskProcess(uint16_t Breath_Amp, uint16_t ApneaTHRE, uint16_t HypopneaTHRE, uint16_t NormalTHRE, float TargetPressure);
void EventDetectionTaskInit(int SamplePerSecond);
// uint8_t EventDetectionTaskProcess(uint16_t Breath_Amp, uint16_t ApneaTHRE, uint16_t HypopneaTHRE, uint16_t NormalTHRE, uint16_t LeakFlow);
uint8_t EventDetectionTaskProcess(uint16_t Breath_Amp, uint16_t BreathTemplateAmplitude, uint16_t LeakFlow);

// extern TypeAHI_info AHI_info;

// extern uint8_t AHI_DECT_S;
// extern uint16_t Count_FlowLimitaiton;
#endif
