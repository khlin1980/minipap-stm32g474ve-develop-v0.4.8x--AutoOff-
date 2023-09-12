#include "Swing.h"

#include <math.h>

extern structPQCurve PQ_Curve;
//extern uint16_t MaskFlow;
//extern uint16_t BreathBaseline;
extern float BreathBaseline;

// extern uint16_t NormalBreathCompare;
// extern uint16_t HyponeaBreathCompare;
// extern uint16_t ApneaBreathCompare;
//extern uint16_t BreathAmplitude;
extern uint8_t BreathPhase;

float TreatmentP;
uint8_t PVA_Status = 0;
float Ratio_Bound = 0.3; // 0.25
float FlowRatio = 0;
float MAX_Flow = 0;

//extern uint16_t FOT_Enable;
//extern unsigned int LeakFlowBaselineCalibration[3];
//extern uint8_t BreathStatus;

float SWING_CONTROL()
{
    float swing_P = 0;

    // float delta_q = (float)pq->FlowLPM_M0 - (float)pq->RoughBaseline;
    float delta_q = PQ_Curve.FlowLPM - PQ_Curve.RoughBaseline;
    float BreathFrequency = PQ_Curve.BreathFrequency;
    float SwingFactor = 0.01875f;
    /***************
          10bpm
    ******************/
    if (BreathFrequency < 14)
    {
        SwingFactor = 0.01875;
    }
    /***************
        15bpm
    ******************/
    else if ((BreathFrequency >= 14) && (BreathFrequency < 18))
    {
        if (TreatmentP <= 60) // 4cmH2O
        {
            SwingFactor = 0.01875;
        }
        else if ((TreatmentP > 60) && (TreatmentP < 90)) // 8cMH2O
        {          
            SwingFactor = 0.0187;      
        }
        else if ((TreatmentP >= 90) && (TreatmentP < 140)) // 12cmH2O
        {
            SwingFactor = 0.022;
        }
        else if ((TreatmentP >= 140) && (TreatmentP < 180)) // 16cmH2O
        {
            SwingFactor = 0.01875;
        }
        else if (TreatmentP >= 180) // 20cmH2O
        {
            SwingFactor = 0.03;
        }
    }
    /***************
        20bpm
    ******************/
    else if (BreathFrequency >= 18)
    {
        if (TreatmentP <= 60) // 4cmH2O
        {   
            SwingFactor = 0.009375;      
        }
        else
        {
            SwingFactor = 0.01875;
        }
    }

    swing_P = delta_q * SwingFactor;

    return swing_P;
}

float PVA_CONTROL(float PVALevel)
{
  static float CycleCounter = 0;
  static float TriggerCounter = 0;

  float Cn = PVALevel * 10;

  if (TreatmentP - Cn < 40)
  {
    Cn = TreatmentP - 40;
  }

  if (Cn == 0)
    return 0;

  float PVAP = 0;
  float CycleRatio = 0;

  switch (PVA_Status)
  {
  case 0: // Limitation

    //
    if (PQ_Curve.FlowLPM > MAX_Flow)
      MAX_Flow = PQ_Curve.FlowLPM;

    // MAX_Flow = IMAX(PQ_Curve.FlowLPM, MAX_Flow);

    CycleRatio = (float)(PQ_Curve.FlowLPM - BreathBaseline) / (float)(MAX_Flow - BreathBaseline);

    if (CycleRatio <= Ratio_Bound)
    {
      CycleCounter = 0;
      PVA_Status = 1;
    }

    PVAP = 0;

    break;
  case 1: // Cycle
    if (Cn >= 25)
      PVAP = -Cn * (1 - exp(-0.10 * CycleCounter)); // for C3: slow down
    else
      PVAP = -Cn * (1 - exp(-0.15 * CycleCounter)); // for C2:

    CycleCounter++;

    if (CycleCounter >= 100) // at least 0.5 sec (50).  100:1sec
    {
      PVA_Status = 2;
      CycleCounter = 0; //!
    }

    break;
  case 2: // Exhale
    PVAP = -Cn;
    // CycleCounter++;
    // Detect the trigger point:
    // if(PQ_Curve.FlowLPM > (BreathBaseline + 10 + (TargetPressure) / 4) && CycleCounter>50)
    //if (PQ_Curve.FlowLPM > (BreathBaseline + 10 + (TargetPressure) / 4))
    //if (PQ_Curve.FlowLPM > (BreathBaseline + 70 + (TreatmentP) / 4))
    if (PQ_Curve.FlowLPM > (BreathBaseline + 70/10 + (TreatmentP) / 4/10))
    {
      PVA_Status = 3;
      TriggerCounter = 0;
      MAX_Flow = PQ_Curve.FlowLPM;
    }

    break;

  case 3: // Trigger
    // float factor = exp(-0.1*(float)AscendCnt);
    if (Cn >= 25)
      PVAP = -Cn * exp(-0.10 * TriggerCounter);
    else
      PVAP = -Cn * exp(-0.15 * TriggerCounter);

    TriggerCounter++;

    if (TriggerCounter >= 70) // at least 0.7s
      PVA_Status = 0;

    if (PQ_Curve.FlowLPM > MAX_Flow)
      MAX_Flow = PQ_Curve.FlowLPM;

    break;

  default:
    break;
  }

  return PVAP;
}

float DynamicPressureCtrl(float TreatmentPressure, uint8_t PVALevel)
{
   TreatmentP = TreatmentPressure;
   
   if(PVALevel>0)
     return PVA_CONTROL(PVALevel);
   else
     return SWING_CONTROL();
     
}