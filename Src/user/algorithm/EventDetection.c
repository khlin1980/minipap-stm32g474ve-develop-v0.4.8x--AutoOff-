
// #include "stm32f4xx_hal.h"
// #include "main.h"
#include <stdint.h>
#include <string.h>
//#include <stdbool.h>
#include "EventDetection.h"
#include "parameter.h"

extern uint8_t isApnea;
extern uint16_t SnoringIndex;
extern uint16_t SnoringTemplate;
extern uint8_t BreathPhase;
extern uint8_t isApneaO;
//uint16_t isApneaO;
extern float FLIndex;
uint16_t NormalBreathCompare;
uint16_t HyponeaBreathCompare;
uint16_t ApneaBreathCompare;
uint16_t FLBreathCompare;

enum AHIDetection_Event
{
    AHI_Event_NoEvent,             // 0
    AHI_Event_SufficeADuration,    // 1
    AHI_Event_NotSufficeADuration, // 2
    AHI_Event_SufficeHDuration,    // 3
    AHI_Event_NotSufficeHDuration, // 4
};

enum AHIDetection_State
{
    AHI_State_WaitBreathAmplitude,        // 0
    AHI_State_DetectingApnea,             // 1
    AHI_State_DetectingHypopnea,          // 2
    AHI_State_EventDetected,              // 3
    AHI_State_BreathStatusApnea,          // 4
    AHI_State_BreathStatusHypopnea,       // 5
    AHI_State_BreathStatusFlowLimitation, // 6
    AHI_State_EventRecord,                // 8
};

enum EventTypes
{
    EventType_OSA = 0x01, // 0
    EventType_CSA = 0x02,
    EventType_Hypopnea = 0x04,      // 1
    EventType_FlowLimitation = 0x08 // 2
};

uint8_t EventStatus = 0;
uint8_t EventType;

uint8_t AHI_DECT_S = AHI_State_WaitBreathAmplitude;

uint8_t AHI_DECT_E; // = AHI_Event_BreathAmpInNTHRE;

uint8_t BreathStatus = BreathStatus_NormalBreath;

TypeAHI_info AHI_info;

uint16_t AHItimeout = 0; // SM timer
uint16_t Count_Apnea;
uint16_t Count_Hypopnea;
// uint8_t Count_Snore;		  //Hank add
uint16_t Count_FlowLimitaiton; // Mei add

// uint16_t AHI_MixInA_Dur = 0;
// uint16_t AHI_MixInH_Dur = 0;

// uint16_t Hevent_Dur = 0;
// uint16_t Aevent_Dur = 0;

// uint8_t OccurOSA;
// uint8_t OccurCSA;
// uint8_t OccurHypopnea;

uint8_t StableNormalCount = 0;

unsigned int Curr_Count_state = 0; // initil;1=apnea;2=hypopnea
unsigned int Prev_Count_state = 0; // initil;1=apnea;2=hypopnea
uint8_t Status_Change_Count = 0;

float FL_Thre = 0.15; // 0.4

unsigned int ui_FLDetectCD = 0;
unsigned int Aovertime = 0;

static int SampleRate_Count;

// int ContinuesApnea = 0;
// int InterruptApneaTimes = 0;

void AHI_CDC()
{
    // CDC_Cnt = 0;
#if 0 
    memset(CDC_Buffer, 0, sizeof(CDC_Buffer));
    sprintf(CDC_Buffer, "BS=%d, State=%d\r\n",             
            BreathStatus,
            AHI_DECT_S);

    CDC_Transmit_FS((uint8_t *)CDC_Buffer, sizeof(CDC_Buffer)); //AUTO_S->MaskFlow
#endif
}

void UpdateThreshold(uint16_t BreathTemplateAmplitude)
{
    float NormalThreshold = 0.6;
    float ApneaThreshold = 0.2;     // 0.1;
    float HypopneaThreshold = 0.43; // 0.53
    float FlowLimitThreshold = 0.8; // 0.8?
    // add issue  hypopnea misjudgemnet to normal breath in 7cmH2O modify 20170405

    NormalBreathCompare = (uint16_t)(BreathTemplateAmplitude * NormalThreshold);

    // ApneaThreshold = 0.2;
    // HypopneaThreshold = 0.43; // 0.53

    HyponeaBreathCompare = (uint16_t)(BreathTemplateAmplitude * HypopneaThreshold);
    ApneaBreathCompare = (uint16_t)(BreathTemplateAmplitude * ApneaThreshold);
    FLBreathCompare = (uint16_t)(BreathTemplateAmplitude * FlowLimitThreshold);
    //  RESET_APNEA_THRESHOLD = Template >> 1;
    //  RESET_HYPOPNEA_THRESHOLD = HyponeaBreathCompare + (Template - HyponeaBreathCompare) >> 1;
}

void EventDetectionTaskInit(int SamplePerSecond)
{
    ui_FLDetectCD = 0;
    // Aovertime = 0;
    // AHI_info.First_Count_In1Min = 0;
    StableNormalCount = 0;
    BreathStatus = BreathStatus_NormalBreath;
    AHI_DECT_S = AHI_State_WaitBreathAmplitude;
    // AHItimeout = 0; // SM timer
    // Hevent_Dur = 0;
    // Aevent_Dur = 0;
    EventStatus = 0;

    // EventTypeFlag = 0;

    // Count_Apnea = 0;
    // Count_Hypopnea = 0;
    Count_FlowLimitaiton = 0;

    /*
    AHI_info.IndexOfOSA = 0;
    AHI_info.IndexOfHypopnea = 0;
    AHI_info.IndexOfFL = 0;
    AHI_info.IndexOfCSA = 0;

    AHI_info.EventHappenedCount = 0;
    AHI_info.EventCount = 0;
    memset(AHI_info.EventTypes, 0, sizeof(AHI_info.EventTypes));
    */

    memset(&AHI_info, 0, sizeof(AHI_info));

    SampleRate_Count = SamplePerSecond;
}

void DetectingApneaInit()
{
    Count_Apnea = 0;
    Count_Hypopnea = 0;
    AHItimeout = 0;
    Aovertime = 0;
    isApneaO = 2; // normal(init)
    Curr_Count_state = 0;
    Prev_Count_state = 0;
    Status_Change_Count = 0;
}

void DetectingHypopneaInit()
{
    Count_Apnea = 0;
    Count_Hypopnea = 0;
    AHItimeout = 0;
}

void CheckStateEvent(uint8_t StateEvent)
{
    if (StateEvent == AHI_Event_NoEvent)
        return;

    switch (StateEvent)
    {
    case AHI_Event_SufficeADuration:
        EventType = (isApneaO) ? EventType_CSA : EventType_OSA;
        AHI_DECT_S = AHI_State_EventDetected;
        break;

    case AHI_Event_SufficeHDuration:
        EventType = EventType_Hypopnea;
        AHI_DECT_S = AHI_State_EventDetected;
        break;

    case AHI_Event_NotSufficeADuration:
        AHI_DECT_S = AHI_State_WaitBreathAmplitude;
        break;

    case AHI_Event_NotSufficeHDuration:
        AHI_DECT_S = AHI_State_WaitBreathAmplitude;
        break;

    default:
        break;
    }
}

// uint8_t EventDetectionTaskProcess(uint16_t Breath_Amp, uint16_t ApneaTHRE, uint16_t HypopneaTHRE, uint16_t NormalTHRE, uint16_t LeakFlow)
uint8_t EventDetectionTaskProcess(uint16_t Breath_Amp, uint16_t BreathTemplateAmplitude, uint16_t LeakRate)
{
    ui_FLDetectCD++; // 20200508
    // FlowLimitationStartTime++; //20200508
    /// execute AHI SM
    UpdateThreshold(BreathTemplateAmplitude);

    //->NormalThresholdCompare:??
    // float NormalThresholdForA = (float)(HyponeaBreathCompare)*1.5f; // 72%
    // float NormalThresholdForH = (float)(HyponeaBreathCompare)*1.5f; // 72%
    float NormalThresholdForA = (float)(NormalBreathCompare); // 72%
    float NormalThresholdForH = (float)(NormalBreathCompare); // 72%

    switch (AHI_DECT_S)
    {
    case AHI_State_WaitBreathAmplitude:

        if (ui_FLDetectCD >= 5 * SampleRate_Count)
        {
            ui_FLDetectCD = 5 * SampleRate_Count;
        }

        if (Breath_Amp <= ApneaBreathCompare || isApnea == 1)
        {
            // AHI_DECT_E = AHI_Event_BreathAmpInATHRE;
            AHI_DECT_S = AHI_State_DetectingApnea;
            DetectingApneaInit();
        }
        else if ((Breath_Amp <= HyponeaBreathCompare && Breath_Amp > ApneaBreathCompare))
        {
            // AHI_DECT_E = AHI_Event_BreathAmpInHTHRE;
            AHI_DECT_S = AHI_State_DetectingHypopnea;
            DetectingHypopneaInit();
        }
        else
        {
            /*
            bool bSnoring = (SnoringIndex >= SnoringTemplate) &&
                            (LeakFlow <= 300) &&
                            (TargetPressure < 100);
            */
            uint8_t bSnoring = (SnoringIndex >= SnoringTemplate) && (LeakRate <= 30);

            /*
            bool bFlowLimitation = (new_FLIndex >= FL_Thre) &&
                                (LeakFlow < 600) &&
                                (ui_FLDetectCD >= 5 * SampleRate_Count) &&
                                (BreathAmplitude > HyponeaBreathCompare) &&
                                (BreathAmplitude < NormalBreathCompare * 1.34);
            */
            uint8_t bFlowLimitation = (FLIndex >= FL_Thre) &&
                                   (LeakRate < 60) &&
                                   (ui_FLDetectCD >= 5 * SampleRate_Count) &&
                                   (Breath_Amp > HyponeaBreathCompare) &&
                                   //(Breath_Amp < NormalBreathCompare * 1.34);
                                   (Breath_Amp < FLBreathCompare);

            // reset index:
            SnoringIndex = 0;
            FLIndex = 0;

            if (bSnoring || bFlowLimitation)
            {
                Count_FlowLimitaiton++;

                if (Count_FlowLimitaiton >= 2) // >= 3
                {
                    EventType = EventType_FlowLimitation;
                    AHI_DECT_S = AHI_State_EventDetected;
                }
            }
        }

        break;

    case AHI_State_DetectingApnea:
        // action: count apnea duration
        // tiime out 10 sec
        if (LeakRate >= 30)
        {
            AHI_DECT_S = AHI_State_WaitBreathAmplitude;
            break;
        }

        AHI_DECT_E = AHI_Event_NoEvent;

        AHItimeout++;

        if (isApnea == 1)
        { // apnea threshold to 0.2 (ApneaTHRE*1.25)0.3;
            Aovertime = 0;
            Curr_Count_state = 1;
            // AHItimeout++;
            Count_Apnea = Count_Apnea + 1;
        }
        //else if (Breath_Amp <= HyponeaBreathCompare * 1.2f)
        else if (Breath_Amp <= HyponeaBreathCompare * 1.2f && Breath_Amp > ApneaBreathCompare)
        {
            Aovertime = 0;
            Curr_Count_state = 2;
            // AHItimeout++;
            Count_Hypopnea = Count_Hypopnea + 1;
        }
        else if (Breath_Amp > HyponeaBreathCompare * 1.2f)
        { // normal breath in 10 second
            Aovertime++;

            if (Aovertime >= 3)
            {
                AHI_DECT_E = AHI_Event_NotSufficeADuration;
            }
            else
            {
                Count_Apnea = Count_Apnea + 1;
            }
        }
        else //isApnea=0 & Breath_Amp < ApneaBreathCompare; caused by NoBreatCnt.
        {
            AHI_DECT_E = AHI_Event_NotSufficeADuration;
        }
        // add new detect apnea function in 20190807
        if (AHItimeout == 1 * SampleRate_Count)
        {
            Prev_Count_state = Curr_Count_state;
            Status_Change_Count = 0;
        }
        else
        { // AHITimeout>1
            if (Prev_Count_state != Curr_Count_state)
                Status_Change_Count = Status_Change_Count + 1;

            Prev_Count_state = Curr_Count_state;
        }

        if (AHItimeout == 10 * SampleRate_Count)
        {
            if (Count_Apnea >= 10 * SampleRate_Count)
            {
                AHI_DECT_E = AHI_Event_SufficeADuration;
            }
            else if (Status_Change_Count >= 4)
            {
                AHI_DECT_E = AHI_Event_SufficeHDuration;
            }
            else if (Status_Change_Count < 4)
            { // stable apnea
                if (Count_Apnea >= Count_Hypopnea)
                {
                    AHI_DECT_E = AHI_Event_SufficeADuration;
                }
                else
                {
                    AHI_DECT_E = AHI_Event_SufficeHDuration;
                }
            }
            else
            {
                AHI_DECT_E = AHI_Event_NotSufficeADuration;
            }
        }

        CheckStateEvent(AHI_DECT_E);

        break;

    case AHI_State_DetectingHypopnea:
        // action: count hypopnea duration
        // tiime out 10 sec
        // Check Leak state:
        if (LeakRate >= 30)
        {
            AHI_DECT_S = AHI_State_WaitBreathAmplitude;
            break;
        }

        AHI_DECT_E = AHI_Event_NoEvent;

        AHItimeout++;

        // Check_AHI_state=2;
        if (Breath_Amp <= HyponeaBreathCompare && (isApnea == 0))
        { // hypopnea threshold to 0.5
            Count_Hypopnea = Count_Hypopnea + 1;
        }
        else if (Breath_Amp <= ApneaBreathCompare * 1.2 || isApnea == 1)
        { // H TO A Detection add 20161223
            Count_Apnea = Count_Apnea + 1;
        }
        else if (Breath_Amp > HyponeaBreathCompare)
        { // normal breath in 10 second
            AHI_DECT_E = AHI_Event_NotSufficeHDuration;
        }

        if (AHItimeout == 10 * SampleRate_Count)
        {
            if (Count_Hypopnea >= 10 * SampleRate_Count || Count_Hypopnea > Count_Apnea)
            {
                AHI_DECT_E = AHI_Event_SufficeHDuration;
            }
            else if (Count_Hypopnea < Count_Apnea)
            {
                AHI_DECT_E = AHI_Event_SufficeADuration;
            }
            else
            {
                AHI_DECT_E = AHI_Event_NotSufficeHDuration;
            }
        }

        CheckStateEvent(AHI_DECT_E);

        break;

    case AHI_State_EventDetected:

        EventStatus = 0;

        switch (EventType)
        {
        case EventType_OSA:
            EventStatus |= EventType_OSA;
            BreathStatus = BreathStatus_OSA;
            AHI_DECT_S = AHI_State_BreathStatusApnea;
            break;

        case EventType_CSA:
            EventStatus |= EventType_CSA;
            BreathStatus = BreathStatus_CSA;
            AHI_DECT_S = AHI_State_BreathStatusApnea;
            break;

        case EventType_Hypopnea:
            EventStatus |= EventType_Hypopnea;
            BreathStatus = BreathStatus_Hypopnea;
            AHI_DECT_S = AHI_State_BreathStatusHypopnea;
            break;

        case EventType_FlowLimitation:
            EventStatus |= EventType_FlowLimitation;
            BreathStatus = BreathStatus_FlowLimitation;
            AHI_DECT_S = AHI_State_BreathStatusFlowLimitation;
            break;

        default:
            break;
        }

        AHI_info.EventHappenedCount++;

        Count_Apnea = 0;
        Count_Hypopnea = 0;

        break;

    case AHI_State_BreathStatusApnea:
        // event still not finish
        if (Breath_Amp > NormalThresholdForA) // normal
        {
            // Breath>noramal And count>=2
            if (BreathPhase == CrossUpPhase)
                StableNormalCount++;
        }
        //! else if (Breath_Amp > ApneaTHRE && Breath_Amp <= HypopneaTHRE) //Hypopnea
        // else if (Breath_Amp > ApneaBreathCompare && Breath_Amp <= NormalThresholdForA) // Hypopnea
        else if (Breath_Amp > ApneaBreathCompare && Breath_Amp <= HyponeaBreathCompare) // Hypopnea
        {
            Count_Hypopnea++;

            // if(Count_Hypopnea==10*SampleRate_Count)
            if (Count_Hypopnea == 16 * SampleRate_Count) // PR036
            {
                AHI_DECT_S = AHI_State_BreathStatusHypopnea;
                BreathStatus = BreathStatus_Hypopnea;
                EventStatus |= EventType_Hypopnea;
                Count_Hypopnea = 0;
            }

            StableNormalCount = 0;
        }
        else // Apnea
        {
            StableNormalCount = 0;
        }

        if (LeakRate > 30 || StableNormalCount >= 2)
        {
            AHI_DECT_S = AHI_State_EventRecord;
        }

        break;

    case AHI_State_BreathStatusHypopnea:

        // if(Breath_Amp > HypopneaTHRE) //Normal
        if (Breath_Amp > NormalThresholdForH) // Normal
        {
            if (BreathPhase == CrossUpPhase)
                StableNormalCount++;
        }
        // else if(Breath_Amp > ApneaTHRE && Breath_Amp<= HypopneaTHRE)//Hypopnea
        // PR036:
        else if (Breath_Amp > ApneaBreathCompare && Breath_Amp <= NormalThresholdForH) // Hypopnea
        {
            StableNormalCount = 0;
        }
        else // maybe Apnea
        {
            Count_Apnea++;

            // if(Count_Apnea==3*SampleRate_Count) //the counter of  no breath amplitude update:13sec = 10sec + 3sec
            if (Count_Apnea == 6 * SampleRate_Count) // PR036
            {
                // KH at 2020/10/21 Bug:Page 40
                if (isApneaO)
                {
                    EventStatus |= EventType_CSA;
                    BreathStatus = BreathStatus_CSA;
                }
                else
                {
                    EventStatus |= EventType_OSA;
                    BreathStatus = BreathStatus_OSA;
                }

                AHI_DECT_S = AHI_State_BreathStatusApnea;

                Count_Apnea = 0;
            }

            // PR036:
            StableNormalCount = 0;
        }

        if (LeakRate > 30 || StableNormalCount >= 2)
        {
            AHI_DECT_S = AHI_State_EventRecord;
        }

        break;

    case AHI_State_BreathStatusFlowLimitation:
        AHI_DECT_S = AHI_State_EventRecord;
        break;

    case AHI_State_EventRecord:
        // for leakage drop under 30lpm from above 35lpm
        if (EventStatus & EventType_FlowLimitation)
        {
            AHI_info.IndexOfFL++;
            AHI_info.EventTypes[AHI_info.EventCount++] = 4;
            // add by KH 2020/10/21: reset the time interval of Flow Limitation
            ui_FLDetectCD = 0;
        }
        else if (EventStatus & EventType_OSA)
        {
            AHI_info.IndexOfOSA++;
            AHI_info.EventTypes[AHI_info.EventCount++] = 1;
        }
        else if (EventStatus & EventType_Hypopnea)
        {
            AHI_info.IndexOfHypopnea++;
            AHI_info.EventTypes[AHI_info.EventCount++] = 2;
        }
        else if (EventStatus & EventType_CSA)
        {
            AHI_info.IndexOfCSA++;
            AHI_info.EventTypes[AHI_info.EventCount++] = 5;
        }

        AHI_DECT_S = AHI_State_WaitBreathAmplitude;

        BreathStatus = BreathStatus_NormalBreath;

        EventStatus = 0;

        StableNormalCount = 0;

        Count_FlowLimitaiton = 0;

        break;

    default:
        break;
    }

    // AHI_CDC();

    return BreathStatus;
}
