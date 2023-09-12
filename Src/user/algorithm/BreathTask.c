#include <stdint.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "BreathTask.h"
#include "FlowLimitation.h"
#include "FlowProcess.h"
#include "DynamicTemplate.h"
#include "EventDetection.h"

extern float BreathBaseline;
extern uint16_t BreathAmplitude;
extern uint16_t BreathFrequency;
extern uint8_t BreathPhase;
extern float BreathBaseline;
extern float RoughBaseline;
extern float LeakBaseline;
extern float FlowRateM0;
extern float FlowRateM1;

extern uint16_t MaskFlow;
extern uint16_t BreathTemplate;

void ApneaDetectionV2(void);
// void DynamicTemplateV2(struct Systems *system);
void DynamicTemplateV2();

uint8_t FL_Segment = 1;
float FLIndex = 0;

// Snoring
float Breath_Start_Amp = 0;
uint16_t IgnoreSnoringCnt = 0;
float SnoringDetectionAmp = 0;
uint16_t TempSnoringLevel = 0;
uint16_t SnoringLevel = 0;
uint16_t SnoringIndex = 0;
uint16_t SnoringBreathCnt = 0;
uint8_t SnoringDetectionCnt = 0;
uint16_t SnoringTemplate = 100;
uint16_t NormalSnoringIdx = 0;
uint32_t NormalSnoringSum = 0;
uint8_t SnoringNoUpdateCnt = 0;

// structPVInfo PV_info1;
structPQCurve PQ_Curve;
// deen flow box paratmeter
uint8_t PressureCalibration[3] = {36, 38, 41};
//unsigned int LeakFlowBaselineCalibration[3] = {3139, 3221, 3312};
uint16_t LeakFlowBaselineCalibration[3] = {14, 22, 31};

// uint8_t BreathStatus;

// �NBreathTask���ϥΪ��Ѽƪ�l��
void BreathTaskInit(void)
{
  FlowRateInit();

  DynamicTemplateInit();

  EventDetectionTaskInit(100);
  
  //  Snoring
  Breath_Start_Amp = 0;
  IgnoreSnoringCnt = 0;
  SnoringDetectionAmp = 0;
  TempSnoringLevel = 0;
  SnoringLevel = 0;

  // SnoringPressure = 0;
  SnoringDetectionCnt = 0;
  SnoringTemplate = 100;
  NormalSnoringSum = 0;
  SnoringNoUpdateCnt = 0;
  // Detected_AHI_Leak_Status = AHIDetectedIn_NonLeak;
  // NoBreathCnt = 0;
  // SmallBreath = 0;
  //  isApneaO = 2;
  //   SWING.rpm = 15;
  // PQ_Curve.BreathFrequency = 15;
  FL_Segment = 0;
}
// �����e���I�y�q�T�� �ΨӰ��@���B�G���L���B��
uint16_t transformFlow(const uint16_t rawFlow)
{
  if (rawFlow <= 32767)
    return rawFlow + 32767;

  return 32768 + rawFlow - 65536;
}

float FlowTransferV3(uint16_t DPADC)
{
  // static float LeakBaseline = 0;

  // PQ_Curve.FlowOffSet = 32766;
  float FlowOffSet = 32766;

  // float DP = (float)PQ_Curve.Flow_ADC - FlowOffSet;
  float DP = (float)DPADC - FlowOffSet;

  float absDP = sqrt(fabs(DP));

  float FlowRate = absDP * absDP * 0.0081 + absDP * 1.0776;

  if (DP <= 0)
    FlowRate = -FlowRate;
  
  return FlowRate*2;

  //float FlowRateLPM = 3000 + FlowRate * 10 * 2;

  //----Leak------

  // float LeakTotalWeight = 300;

  // PQ_Curve.FlowLPM_M1 = LowPassFilter(PQ_Curve.FlowLPM);
  // PQ_Curve.FlowLPM_M0 = NotchFilter(PQ_Curve.FlowLPM_M1);

  // LeakBaseline = LeakBaseline + (PQ_Curve.FlowLPM - LeakBaseline) / LeakTotalWeight;

  // PQ_Curve.LeakBaseline = LeakBaseline;

  //return FlowRateLPM;
}

///////ApneaDiff filter///////
uint8_t isApnea;// = 0;
//float ApneaDiff = 0;
//uint16_t ApneaDuration = 0;
//uint16_t FOT_Enable = 0;
float ApneaDiff = 0;
float ApneaBaseline = 0;

void ApneaDetectionV2(void)
{
  //static float ApneaBaseline = 0;
  // static float ApneaDiff = 0;

  float TotalWeight = 150;//200.0f;
  // uint32_t Averaged = 0;
  //  Moving average with 300 points ==> Breath baseline
  //  filteredLPM = NotchFilter(PQ_Curve.FlowLPM_M0);
  // ApneaBaseline = (ApneaBaseline1 * (TotalWeight - 1) + PQ_Curve.FlowLPM_M0) / (float)TotalWeight;
  ApneaBaseline = ApneaBaseline + (PQ_Curve.FlowLPM_M0 - ApneaBaseline) / TotalWeight;
  // Averaged = (LastedApneaBaseline * (TotalWeight - 1) + filteredLPM * 1000) / TotalWeight;
  // LastedApneaBaseline = Averaged;
  // ApneaBaseline = Averaged / 1000;
  ApneaDiff = PQ_Curve.FlowLPM_M0 - ApneaBaseline;

  ApneaDiff = fabs(ApneaDiff);

  // Setting threshold value of apnea different is about 36.

  // add FOT in 20191101
  /*
  if (ApneaDiff <= 50/10) // 70
  {
    isApnea = 1;
    ApneaDuration++;
    if (ApneaDuration == 400)
    {
      FOT_Enable = 1; // FOT(&sys);//FOT on
    }
  }
  else
  {
    isApnea = 0;
    ApneaDuration = 0;
    FOT_Enable = 0; // FOT off
  }
  */
  
  isApnea = (ApneaDiff <= 50/10);
}

// Snoring detection 30 Hz High Pass Filter
float SnoringHighPassFilter(float SnoreingSensorPressure)
{
  float Coef_a[4] = {1, -0.577240524806303, 0.421787048689562, -0.0562972364918427};                // adjustable
  float Coef_b[4] = {0.256915601248463, -0.770746803745390, 0.770746803745390, -0.256915601248463}; // adjustable
  // float Temp_Data = 0;
  static float SnoringData_Buffer[4][2] = {0};
  // unsigned char j = 0;
  // uint16_t FilterPressure = 0;

  SnoringData_Buffer[0][0] = SnoreingSensorPressure;

  float Temp_Data = Coef_b[0] * (SnoreingSensorPressure);

  for (int j = 1; j < 4; j++)
  {
    Temp_Data = Temp_Data + Coef_b[j] * SnoringData_Buffer[j][0] - Coef_a[j] * SnoringData_Buffer[j][1];
  }

  SnoringData_Buffer[0][1] = Temp_Data;

  for (int j = 1; j < 4; j++)
  {
    SnoringData_Buffer[3 - j + 1][0] = SnoringData_Buffer[3 - j][0];
    SnoringData_Buffer[3 - j + 1][1] = SnoringData_Buffer[3 - j][1];
  }

  float FilterPressure = (Temp_Data * Temp_Data * 10);
  // FilterPressure = (Temp_Data*Temp_Data*10);
  return FilterPressure;
}

// void SnoringDetection(struct Systems *system)
void SnoringDetectionV2(float SensorPressure) // SensorPressure = system->SensorP;
{
  // ���o���O���W�T��
  // SnoringPressure = SnoringHighPassFilter(system->SensorP);
  uint16_t SnoringPressure = (uint16_t)(SnoringHighPassFilter(SensorPressure));

  //uint16_t LeakFlow = MaskFlow * 10; // refer to PressureTask.c: LeakFlow = MaskFlow * 10;

  // uint16_t TargetPressure = (uint16_t)system->TargetP;

  switch (BreathPhase)
  {
  case UpperPhase:
    if (MaskFlow < 30)
    { // Large leak(30 lpm) do not detect snoring
      // �b�l��۶}�l�p��snoring index
      if (IgnoreSnoringCnt == 0)
      {
        Breath_Start_Amp = BreathBaseline;
      }

      if (IgnoreSnoringCnt < 25)
        SnoringDetectionAmp = PQ_Curve.FlowLPM_M0 - Breath_Start_Amp;

      IgnoreSnoringCnt++;
      // �e��0.25���������p �B�I�l�_�T�����j�~�|�}�l�p��snoring index
      if (IgnoreSnoringCnt >= 25)
      {
        if (SnoringDetectionAmp > 40)
        {
          if ((PQ_Curve.FlowLPM_M0 - Breath_Start_Amp) > 40) // XT50
          {
            TempSnoringLevel = TempSnoringLevel + SnoringPressure;

            if (TempSnoringLevel > 60000)
            {
              TempSnoringLevel = 60000;
            }
            SnoringBreathCnt++;
          }
        }
        else
        {
          IgnoreSnoringCnt = 0;
        }
      }
    }
    else
    {
      TempSnoringLevel = 0;
    }
    SnoringLevel = 0;
    // SnoringIndex = 0;
    break;
  case CrossDownPhase:
    // �l��۵��� ����p��
    if (SnoringBreathCnt <= 400)
    { // Snoring�����ɶ��j��4���A�h���p��(�קK�]���L�I�l�i�~�P)
      // SnoringLevel = TempSnoringLevel/100;
      SnoringLevel = TempSnoringLevel / 5; // IX

      SnoringBreathCnt = 0;
      /*
      if (TargetPressure < 80)
      {
        // SnoringTemplate = final_P;
        // SnoringTemplate = 30+(system->FinalP/10);//XT50
        SnoringTemplate = 20 + (system->FinalP / 10); // IX
      }
      else
      {
        // SnoringTemplate = final_P*0.9;
        // SnoringTemplate = 30+(system->FinalP/10);//XT50
        SnoringTemplate = 20 + (system->FinalP / 13); // IX
      }
      */
      SnoringTemplate = 30; // IX
    }
    else
    {
      SnoringLevel = 0;
    }
    break;
  case CrossUpPhase:
    // ��@�I�l���� �N���I�l�����G�e��AHI detection
    // Initialization after send SnoringLevel
    //if (LeakFlow < 300)
    if(MaskFlow < 30)
    {
      SnoringIndex = SnoringLevel;
    }
    else
    {
      SnoringIndex = 0;
    }
    // RecordSnoring = SnoringIndex;
    TempSnoringLevel = 0;
    IgnoreSnoringCnt = 0;
    SnoringBreathCnt = 0;

    break;

  default:

    break;
  }
}

void FLDetectionV2(void)
{
  static float TriggerPointValue = 0;
  static uint8_t FL_Cnt = 0;
  static uint8_t FL_NUM = 0;
  static float FLFlow[225];

  if (BreathPhase == CrossUpPhase)
  {
    FL_Segment = 1;
    TriggerPointValue = BreathBaseline;

    FLFlow[0] = PQ_Curve.FlowLPM_M0 - TriggerPointValue;
    FL_NUM = 1;
    FL_Cnt = 0;
  }

  // start to sample waveform
  if (FL_Segment == 1)
  {
    FL_Cnt++; // each 3 data points mod 1 100Hz--->33Hz

    if (FL_Cnt % 3 == 0) //&& PQ_Curve.FlowLPM_M0 > (BreathBaseline + 20))
    {
      FLFlow[FL_NUM] = PQ_Curve.FlowLPM_M0 - TriggerPointValue;
      FL_NUM++;

      if (FL_NUM >= 25)
      {
        float Flowdiffer1 = FLFlow[FL_NUM - 1] - FLFlow[0];
        float Flowdiffer2 = FLFlow[FL_NUM - 2] - FLFlow[0];

        if (Flowdiffer1 * Flowdiffer2 <= 0)
        {
          FL_Segment = 0;
          FLIndex = RecognizeFlowLimitation(FLFlow, FL_NUM);
        }
      }

      if (FL_NUM >= 225)
        FL_Segment = 0;
    }
  }
}

// void BreathTaskProcess(struct Systems *system)

//void BreathTaskProcess(float Pressure, uint16_t DP)
void BreathTaskProcess(float Pressure, float flow)
{
  PQ_Curve.FlowLPM = flow;//FlowTransferV3(DP);

  /*
  Differential(PQ_Curve.FlowLPM_M0);
  // CheckPhase();
  CheckPhaseV2();
  // CheckPhaseV2();
  // PeakValleyDetectionV2();
  // PeakValleyDetection();
  PeakValleyDetectionV2();

  // BaselineStateTransition(BreathPhase);

  BaselineCalculation();
  */
  FlowRateProcess(PQ_Curve.FlowLPM);

  PQ_Curve.FlowLPM_M0 = FlowRateM1;
  PQ_Curve.FlowLPM_M1 = FlowRateM0;
  PQ_Curve.LeakBaseline = LeakBaseline;
  PQ_Curve.BreathFrequency = BreathFrequency;
  PQ_Curve.RoughBaseline = RoughBaseline;
  // PQ_Curve.

  // PQ_Curve.

  // PQ_Curve.BreathFrequency

  // DynamicTemplate(system);
  // if (BreathPhase == CrossUpPhase)
  DynamicTemplateV3(BreathAmplitude, BreathPhase);
  
  FLDetectionV2();
  // SnoringDetection(system);
  //SnoringDetectionV2(Pressure);
  ApneaDetectionV2();

  //uint16_t LeakFlow = MaskFlow * 10;

  // BreathStatus = EventDetectionTaskProcess(BreathAmplitude, BreathTemplate, LeakFlow);
  EventDetectionTaskProcess(BreathAmplitude, BreathTemplate, MaskFlow);
}

uint8_t AutoOnDetect(uint16_t DP)
{
  PQ_Curve.FlowLPM = FlowTransferV3(DP);
  
  FlowRateProcess(PQ_Curve.FlowLPM);

  PQ_Curve.FlowLPM_M0 = FlowRateM1;
  PQ_Curve.FlowLPM_M1 = FlowRateM0;
  //PQ_Curve.LeakBaseline = LeakBaseline;
  //PQ_Curve.BreathFrequency = BreathFrequency;
  //PQ_Curve.RoughBaseline = RoughBaseline;
  // PQ_Curve.

  // PQ_Curve.

  // PQ_Curve.BreathFrequency

  // DynamicTemplate(system);
  // if (BreathPhase == CrossUpPhase)
  //DynamicTemplateV3(BreathAmplitude, BreathPhase);
  
  //FLDetectionV2();
  // SnoringDetection(system);
  //SnoringDetectionV2(Pressure);
  ApneaDetectionV2();
  
  return (BreathAmplitude>15);
  //return (ApneaDiff>40);
}