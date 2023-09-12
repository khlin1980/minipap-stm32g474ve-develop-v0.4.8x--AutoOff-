// #include "stm32f4xx_hal.h"
// #include "main.h"

#include "Swing.h"
// #include "iXAUTO_AHIDetection.h"
#include "AutoMode.h"
//#include "iXAUTO_DataProcess.h"
// #include "iXAUTO_LeakageCompensate.h"
#include "LeakManagement.h"
#include "PressureTask.h"
#include "parameter.h"
#include <math.h>

/*
#ifndef M_PI
#define M_PI 3.1415926535897932384626433
#endif
*/

//extern int16_t curr_cmH2o;

// uint8_t PressureControlState = PressureControlState_Swing;
// extern uint8_t DisableFOT;

extern structPQCurve PQ_Curve;
extern uint16_t MaskFlow;
//extern uint16_t BreathBaseline;
//extern float BreathBaseline;

// extern uint16_t NormalBreathCompare;
// extern uint16_t HyponeaBreathCompare;
// extern uint16_t ApneaBreathCompare;
//extern uint16_t BreathAmplitude;
extern uint8_t BreathPhase;
//extern uint16_t FOT_Enable;
//extern unsigned int LeakFlowBaselineCalibration[3];
extern uint16_t LeakFlowBaselineCalibration[3];
extern uint8_t BreathStatus;

extern uint8_t isApnea;
uint16_t ApneaCounter;
//uint8_t FOT_Enable;

//uint16_t MaskFlow1;
//+Ann 20190918 - Pressute Task
float RampStopP;

///////FOT Variable////////
uint16_t FOT_Counter = 0;
float FOT_P[400];
float FOT_Q[400];


uint8_t isApneaO = 2;
//int PhaseDelay;

// int leakp1,leakp2;
// int dQ,dP;

/***********************************************************
FUNCTION: PressureTaskInitialize
Decription:  Initial parameters
Argument: config - structure
Return:  N/A
Note:
***********************************************************/
//float RampStopP;

// for PVA:
//uint32_t PVACounter;
//int BreathingCount = 0;
//int PVA_Active = 0;

void PressureTaskInit(struct Systems *system)
{
  if (system->Config.Mode == 0) // CPAP mode
  {
    RampStopP = system->Config.TherapyP;
  }
  else
  {
    RampStopP = system->Config.MinP;
  }

  system->IsDuringRamp = 1;
  system->TargetP = 40;
  system->LargeLeakTime = 0;

  LeakManagementInit(LeakFlowBaselineCalibration); // KH
  
  AutoModeTaskInit(system->Config.MinP, system->Config.MaxP, 100);  
}

float PressureTaskProcessOfTreatment(struct Systems *system)
{
  //float FOTP = 0.0f;

  // uint8_t BreathStatus;

  /*comment by KH at 2020/10/19
    counter10ms++;

    if (counter10ms < 400)
    {
      return; // -2;
    }
    */
  //if (system->Counter10ms < 400)
  //  return system->TargetP;

  // if (system->State == 0) //RAMP;
  if (system->TargetP < RampStopP) // RAMP;
  {
    if (system->Config.RampTime > 0) // Ramp Enable
    {
      if (system->TargetP < system->Config.RampStartP)
      {
        system->TargetP += 0.05; // 0.1; //increment 0.1cmH2O/10ms
      }
      // else if (system->TargetP < RampStopP) //Ramp Time; modified by KH
      else // Ramp Time;
      {
        system->TargetP += (RampStopP - system->Config.RampStartP) / (system->Config.RampTime * 60 * 100);
        //PVACounter++;
      }
    }
    else
    {
      system->TargetP += 0.05;
    }

    system->RecordTargetP = system->TargetP;
  }
  else
  {
    //PVACounter++;

    system->IsDuringRamp = 0; // start to treatment:
    // system->State = 1;
    if (system->Counter10ms >= 30000) // apap
    {                                 // counter10ms< 5min
      // uint16_t LeakFlow = MaskFlow * 10; // for ahi detection and automode function
      //  BreathStatus = EventDetectionTaskProcess(BreathAmplitude, ApneaBreathCompare, HyponeaBreathCompare, NormalBreathCompare, system->TargetP);
      // BreathStatus = EventDetectionTaskProcess(BreathAmplitude, ApneaBreathCompare, HyponeaBreathCompare, NormalBreathCompare, LeakFlow);
      //  AHIInLeakage();
      if (system->Config.Mode == 1) // APAP
        // AutoModeTaskProcess(BreathStatus, &system->TargetP);
        AutoModeTaskProcess(BreathStatus, &system->TargetP, &system->RecordTargetP);
    }
    else
    {
      system->RecordTargetP = system->TargetP;
    }
  }

  
  //LeakManagementTask(PQ_Curve.LeakBaseline, system->TargetP, system->TargetP + system->LeakP, HHStatus.HALL, &MaskFlow, &system->LeakP);
  LeakManagementTask(PQ_Curve.LeakBaseline, system->TargetP, system->TargetP + system->LeakP, 0, &MaskFlow, &system->LeakP);

  if(MaskFlow>=100)
    system->LargeLeakTime++;
  else
    system->LargeLeakTime = 0;
  
  system->PVAP = DynamicPressureCtrl(system->TargetP, system->Config.PVALevel);

  float FOTP = 0;
  
  if(BreathStatus != BreathStatus_OSA && BreathStatus != BreathStatus_CSA)
  {
    if(isApnea)
    {
      FOT_Counter++;
      
      if(FOT_Counter>500)
      {    
        float w = 2 * 3.14157 * 4.0 * FOT_Counter / 100;

        FOTP = 0.75 * sin(w) * 10; // 0.365

        FOT_P[FOT_Counter-501] = system->SensorP;
        FOT_Q[FOT_Counter-501] = PQ_Curve.FlowLPM_M1;
        
        if(FOT_Counter==900)
        {     
          //must enable: GetPhaseDelay() is definied in DataProcess.c
          int phase_delay = 10;//GetPhaseDelay(FOT_Pressure, FOT_FLow, 400); // discuss later
          //PhaseDelay = phase_delay;
          // CSAEvent = (phase_delay >= 12); // 13;11
          isApneaO = (phase_delay >= 12);
          
          FOT_Counter = 0;        
        }
      }  
    }
    else
    {
      FOT_Counter = 0;
    }
  }
  else
  {
    FOT_Counter = 0;
  }
  
  system->FinalP = system->TargetP + FOTP + system->PVAP;// + system->SwingP;;
  //system->FinalP = system->TargetP + system->LeakP + system->PVAP + system->SwingP + FOTP;
  // system->FinalP = system->TargetP + system->PVAP + system->SwingP + FOTP;
  //  system->FinalP = system->TargetP + system->PVAP + system->SwingP + FOTP;
  //  system->FinalP = system->TargetP + system->LeakP + system->PVAP + system->SwingP;
  //SetRPMByFansLaw(system->SensorP, system->FinalP);


  
  return system->FinalP;
}

void PressureTaskProcessOfCalibration(struct Systems *system)
{
  if (system->Counter10ms < 400)
    return;

  system->FinalP = system->TargetP;

  //SetRPMByFansLaw(system->SensorP, system->FinalP, system->TargetP);
}

void PressureTaskProcessOfRunMaskFit(struct Systems *system)
{
  if (system->TargetP < RampStopP) // 10 sec
  {
    system->TargetP += 0.1;
  }

  system->Leakage = MaskFlow;

  system->FinalP = system->TargetP; // + system->LeakP + system->PVA_setP + system->SwingP;
}
