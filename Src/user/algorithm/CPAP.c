#include "CPAP.h"
#include "BreathTask.h"
#include "PressureTask.h"
#include "PressureControl.h"

struct Systems sys;

void CPAPSetConfig(uint8_t Mode, uint8_t RampTime, uint8_t RampStartP, uint8_t PVALevel, uint8_t TreatmentPressure, uint8_t MinPressure, uint8_t MaxPressure)
{
  sys.Config.Mode = Mode;
  sys.Config.MinP = MinPressure;
  sys.Config.MaxP = MaxPressure;
  sys.Config.RampTime = RampTime;
  sys.Config.RampStartP = RampStartP;
  sys.Config.PVALevel = PVALevel;
  sys.Config.TherapyP = TreatmentPressure;
}

void CPAPInit()
{
  sys.Counter10ms = 0;
  BreathTaskInit();
  PressureTaskInit(&sys);
}

void CPAPProcess(int16_t Pressure, float Flow)
{
  BreathTaskProcess(Pressure, Flow);  
  float outPressure = PressureTaskProcessOfTreatment(&sys);
  
  PressureCtrl(outPressure);
  
  sys.Counter10ms++;
}

uint8_t CPAPAutoOn(uint16_t Flow)
{
  return AutoOnDetect(Flow);
}

uint8_t CPAPAutoOff()
{
  return (sys.LargeLeakTime/10>=10);
}