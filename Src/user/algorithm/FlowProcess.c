#include <stdint.h>
#include <math.h>
#include <string.h>
#include "FlowProcess.h"
#include "parameter.h"

// for filter:
float FlowRateM0;
float FlowRateM1;
float NotchFilter(float FlowRate);
float LowPassFilter(float FlowRate);

// for Baseline():
float LeakBaseline;
float RoughBaseline;
float BreathBaseline;
uint8_t BaselineCal_State;
void BaselineStateTransitionV2();
void BaselineEstimation(float FlowRate);

// Leak
uint16_t MaskFlow = 0;

float gain_factor = 10;
// for Differential():
structDiff tempDiff; // = {0};
void Differential(float FlowRate);

// for CheckPhaseV2():
uint8_t CheckPhase_State;       // = CheckPhase_STATE_IDLE;
uint8_t BreathPhase;            // = NoBreathPhase; //be defined in parameter.h
float DynamicValueAdd = 100; // 250;//240;//250;
float DynamicValueSub = 105; // 150;
void CheckPhaseV3(float FlowRate);

// for PeakValleyDetection():
uint16_t BreathAmplitude;
uint16_t BreathFrequency;
uint8_t PV_State; // = PV_State_InspirWaiting;
//structPVInfo PV_info;
int BreathCounter;         // = 0;
int NoBreathCnt;           //
int NoBreathAmplitude = 1; // 20
void PeakValleyDetectionV2(void);
//void PeakValleyStateTransitV3();
void PeakValleyDetectionV3(float FlowRate);

//-------------------------------------------------------------------------------------------------------------
float LowPassFilter(float FlowRate)
{
    static float w[4][3] = {0, 0};
    // Coefficient
    const float a[4][2] = {// a1 a2
                           {-1.33171493774799, 0.447594835879771},
                           {-1.43404024021778, 0.546304683815898},
                           {-1.59529532020433, 0.704665610702544},
                           {-1.78129466840616, 0.893553431712330}};
    const float b[4][3] = {// b0 b1 b2
                           {0.000254216348454991, 0.000238981226441617, 0.000254216348454990},
                           {1, -0.980541916523052, 1.00000000000001},
                           {1, -1.47009218347023, 0.999999999999964},
                           {1, -1.60441568016271, 1.00000000000003}};
    // char k;
    float filtered = FlowRate;

    for (int k = 0; k < 4; k++)
    {
        w[k][0] = filtered - a[k][0] * w[k][1] - a[k][1] * w[k][2];
        filtered = b[k][0] * w[k][0] + b[k][1] * w[k][1] + b[k][2] * w[k][2];
        w[k][2] = w[k][1]; // w(n-2) <- w(n-1)
        w[k][1] = w[k][0]; // w(n-1) <- w(n)
    }

    return filtered;
}

float NotchFilter(float FlowRate)
{
    static float NotchBuffer[3][2] = {0};

    float b[] = {0.8878, -1.7199, 0.8878};
    float a[] = {1.0000, -1.7199, 0.7757};

    // float filtered; // change to local, by dean.

    NotchBuffer[0][0] = FlowRate;

    // float filtered = Coef_b[0]*input;
    float filtered = b[0] * FlowRate;

    for (int j = 1; j < 3; j++)
    {
        filtered = filtered + b[j] * NotchBuffer[j][0] - a[j] * NotchBuffer[j][1];
    }

    NotchBuffer[0][1] = filtered;

    for (int j = 1; j < 3; j++)
    {
        NotchBuffer[2 - j + 1][0] = NotchBuffer[2 - j][0];
        NotchBuffer[2 - j + 1][1] = NotchBuffer[2 - j][1];
    }

    return filtered;
}

void BaselineEstimation(float FlowRate)
{
    /*
    FlowRate: PQ_Curve.FlowLPM
    FlowTransfer(): LeakBaseline = LeakBaseline + (PQ_Curve.FlowLPM - LeakBaseline) / LeakTotalWeight;
    CheckPHase(): RoughBaseline = RoughBaseline + (PQ_Curve.FlowLPM - RoughBaseline) / TotalWeight;
    */
    float TotalWeight = 300;
    
    LeakBaseline = LeakBaseline + (FlowRate - LeakBaseline) / TotalWeight;
    RoughBaseline = RoughBaseline + (FlowRate - RoughBaseline) / TotalWeight;
    // RoughBaseline = LeakBaseline;

    // BaselineCalculation();
}

void BaselineStateTransitionV2()
{
    switch (BreathPhase)
    {
    case DownPhase:
        if (BaselineCal_State == BaselineCal_State_IDLE)
            BaselineCal_State = BaselineCal_State_Detect;
        break;
    case CrossUpPhase:
        if (BaselineCal_State == BaselineCal_State_Detect)
            BaselineCal_State = BaselineCal_State_Confirm;
        break;
    case UpperPhase:
        BaselineCal_State = BaselineCal_State_IDLE;
        break; // Hank add 0508
    case CrossDownPhase:
        BaselineCal_State = BaselineCal_State_IDLE;
        break;
    default:
        break;
    }
}


float tempBreathBaseline = 0;

void BaselineCalculation(float FlowRate)
{
    // FlowRate:PQ_Curve.FlowLPM_M0
    //  tempBreathBaseline: only use in this function
    //static float tempBreathBaseline = 0;

    BaselineStateTransitionV2();

    switch (BaselineCal_State)
    {
    case BaselineCal_State_IDLE:
        break;
    case BaselineCal_State_Detect:
        if ((tempDiff.D2y * tempDiff.NextD2y <= 0) && tempDiff.NextD2y >= tempDiff.D2y)
        {                                  // f''(x) = 0
            tempBreathBaseline = FlowRate; // PQ_Curve.FlowLPM_M0;
        }
        break;
    case BaselineCal_State_Confirm:
        // TODO Modify~~~~~
        // BreathBaseline = FlowLPM_M0;
        if (BreathAmplitude > 100/gain_factor)
        {
            BreathBaseline = (7 * BreathBaseline + 3 * tempBreathBaseline) / 10;
        }
        else
        {
            BreathBaseline = RoughBaseline;
        }
        break;
    default:
        break;
    }
}

void Differential(float FlowRate)
{
    static float TDL[5] = {0};

    TDL[4] = TDL[3];
    TDL[3] = TDL[2];
    TDL[2] = TDL[1];
    TDL[1] = TDL[0];
    TDL[0] = FlowRate;

    tempDiff.Dy = TDL[2] - TDL[3];
    tempDiff.NextDy = TDL[1] - TDL[2];
    tempDiff.D2y = TDL[1] - 2 * TDL[2] + TDL[3];
    tempDiff.NextD2y = TDL[0] - 2 * TDL[1] + TDL[2];
}

void CheckPhaseV3(float FlowRate)
{
    // Note: FlowRate = PQ_Curve_FlowLPM_M0
    // float TotalWeight = 300;
    float DownphaseOffset = 10; // 75

    // uint32_t Averaged = 0;
    //  Moving average with 300 points ==> Breath baseline
    // Averaged = (LastedBaseline2 * (TotalWeight - 1) + PQ_Curve.FlowLPM * 1000) / TotalWeight;
    // LastedBaseline2 = Averaged;

    // RoughBaseline = RoughBaseline + (PQ_Curve.FlowLPM - RoughBaseline) / TotalWeight;

    //uint16_t LeakFlow = MaskFlow * 10; // refer to PressureTask.c: LeakFlow = MaskFlow * 10;
    // modified by KH: PressureControlState is not be used.
    DynamicValueAdd = 100/gain_factor;
    DynamicValueSub = 50/gain_factor;  // 100//90
    DownphaseOffset = 10/gain_factor; // 75;//0

    /*disable//need to check
    if (BreathAmplitude <= HyponeaBreathCompare)
    {
        DynamicValueAdd = 0;
        DynamicValueSub = 0;
    }
    */

    //
    // Start to Check phase until both of flow and baseline signal were stable
    if (FlowRate > RoughBaseline && CheckPhase_State == CheckPhase_STATE_IDLE)
    {
        CheckPhase_State = CheckPhase_STATE_START;
        // CheckPhase_State = CheckPhase_State;
        // Start to check phase until filter flow first below baseline
    }
    else if (CheckPhase_State == CheckPhase_STATE_START)
    {
        CheckPhase_State = CheckPhase_STATE_START;
        // PreCheckPhase = CheckPhase_State;
    }
    else
    {
        CheckPhase_State = CheckPhase_STATE_IDLE;
        // PreCheckPhase = CheckPhase_State;
    }
    // Distinguish breath phase according the relation with baseline and flow
    // �קK���T�z�Z �l��ۤ����I�ݭn��RoughBaseline���@��(25) �R��ۤ����I�ݭn��RoughBaseline�C�@��(-75)
    if (CheckPhase_State != CheckPhase_STATE_START)
        return;

    if (FlowRate >= (RoughBaseline + 25/gain_factor))
    {
        if (BreathPhase == NoBreathPhase || BreathPhase == DownPhase)
        {
            BreathPhase = CrossUpPhase;
            // LastedBaseline2 = LastedBaseline2 - DynamicValueSub * 1000; // �[�J�ʺA�֭� for XT50
            RoughBaseline = RoughBaseline - DynamicValueSub; // �[�J�ʺA�֭� for XT50
        }
        else if (BreathPhase == CrossUpPhase)
        {
            BreathPhase = UpperPhase;
        }
    }
    else if (FlowRate < (RoughBaseline - DownphaseOffset) || (MaskFlow > 30 && FlowRate < (RoughBaseline - 25/gain_factor)))
    {
        if (BreathPhase == NoBreathPhase || BreathPhase == UpperPhase)
        {
            BreathPhase = CrossDownPhase;
            // LastedBaseline2 = LastedBaseline2 + DynamicValueAdd * 1000;
            RoughBaseline = RoughBaseline + DynamicValueAdd;
        }
        else if (BreathPhase == CrossDownPhase)
        {
            BreathPhase = DownPhase;
        }
    }
}

void PeakValleyStateTransitV2()
{
    switch (PV_State)
    {
    case PV_State_InspirWaiting:
        if (BreathPhase == UpperPhase)
            PV_State = PV_State_PeakFinding;
        break;
    case PV_State_PeakFinding:
        if (BreathPhase == CrossDownPhase)
            PV_State = PV_State_PeakChecking;
        break;
    case PV_State_PeakChecking:
        if (BreathPhase == DownPhase)
            PV_State = PV_State_ValleyFinding;
        break;
    case PV_State_ValleyFinding:
        if (BreathPhase == CrossUpPhase)
            PV_State = PV_State_ValleyChecking;
        break;
    case PV_State_ValleyChecking:
        if (BreathPhase == UpperPhase)
            PV_State = PV_State_PeakFinding;
        break;
    default:
        break;
    }
}

void PeakValleyDetectionV3(float FlowRate)
{
    static float PeakValue = 0;
    // FlowRate:PQ_Curve.FlowLPM_M0
    BreathCounter++;
    // NoBreathCnt2++;

    // PeakValleyStateTransit(BreathPhase);
    PeakValleyStateTransitV2();
    // BreathPhase Event transit Peak Valley action
    switch (PV_State)
    {
    case PV_State_PeakFinding:
        // Find Peak
        //if ((tempDiff.Dy * tempDiff.NextDy <= 0) && (tempDiff.D2y < 0) && FlowRate >= PV_info.TempPeak_Val)
        if ((tempDiff.Dy * tempDiff.NextDy <= 0) && (tempDiff.D2y < 0) && FlowRate >= PeakValue)
        {
            // if (FlowLPM_M0>=PV_info.TempPeak_Val){
            //PV_info.TempPeak_Val = FlowRate; // PQ_Curve.FlowLPM_M0;
            PeakValue = FlowRate;
            // PV_info1.TempPeak_Pos = TreatmentDuration;
        }

        // NoBreathCnt = 0;
        //  NoBreathCnt++;
        //   BaselineStateTransition(BreathPhase);
        break;
    case PV_State_PeakChecking:
        // if ((PV_info.TempPeak_Pos - PV_info.Valley_Pos) >= EnableCycle)
        // if ((PV_info.TempPeak_Pos - PV_info.Peak_Pos) >= EnableCycle)
        if (BreathCounter > 200) // check peak
        {
            //PV_info.Peak_Val = PV_info.TempPeak_Val;
            // PV_info1.Peak_Pos = PV_info1.TempPeak_Pos;

            //if (PV_info.Peak_Val >= BreathBaseline) //??
            if (PeakValue >= BreathBaseline) //??
            {
                //BreathAmplitude = PV_info.Peak_Val - BreathBaseline;
                BreathAmplitude = (uint16_t)(PeakValue - BreathBaseline);

                // if (BreathAmplitude > ApneaBreathCompare) // XT50+++++
                //{
                NoBreathCnt = 0;
                
                if (BreathCounter < 1000)
                    BreathFrequency = (uint16_t)(6000 / (float)(BreathCounter) + 0.5f);
                //}
            }
        }

        // Reset:
        PV_State = PV_State_InspirWaiting;
        BreathCounter = 0;
        //PV_info.TempPeak_Val = 0;
        PeakValue = 0;

        break;
    default:
        NoBreathCnt++;
        if (NoBreathCnt > 1000)
        {
            BreathAmplitude = NoBreathAmplitude;
            PV_State = PV_State_InspirWaiting;
        }

        break;
    }

    // BaselineStateTransition(BreathPhase);
}

void FlowRateInit()
{
    //memset(&PV_info, 0, sizeof(structPVInfo));

    // PV_info.TempPeak_Val = 4096;
    BreathPhase = NoBreathPhase;
    // BreathPhaseSwing = NoBreathPhase; // XT50
    PV_State = PV_State_InspirWaiting;

    memset(&tempDiff, 0, sizeof(structDiff));

    NoBreathCnt = 0;
    BreathCounter = 0;

    BreathBaseline = 0; // XT50
    RoughBaseline = 0;
    LeakBaseline = 0;

    BaselineCal_State = BaselineCal_State_IDLE;

    CheckPhase_State = CheckPhase_STATE_IDLE;

    BreathAmplitude = 0;
    BreathFrequency = 0;
}

void FlowRateProcess(float FlowRate) //
{
    FlowRateM0 = LowPassFilter(FlowRate);
    FlowRateM1 = NotchFilter(FlowRateM0);

    BaselineEstimation(FlowRate);

    Differential(FlowRateM1);

    CheckPhaseV3(FlowRateM1);

    BaselineCalculation(FlowRateM1);

    PeakValleyDetectionV3(FlowRateM1);
}
