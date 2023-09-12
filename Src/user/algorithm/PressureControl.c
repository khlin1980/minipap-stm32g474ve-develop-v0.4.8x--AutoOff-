#include "PressureControl.h"
#include "motor_control.h"
#include <stdint.h>

extern int16_t curr_mmH2O;

// Add by KH 2019/11/28
uint32_t GetTargetRPMByFansLaw(float sensor_p, float target_p, uint32_t current_rpm)
{
  if ((sensor_p > 0) && (current_rpm != 0) && (target_p < 400) && (target_p > 0))
  {
    return (uint32_t)(sqrt(target_p / sensor_p) * current_rpm);
  }

  return 8000;
}

/**********************************************************/
// Add by KH 2019/11/28
void SetRPMByFansLaw(float SensorP, float TargetP, uint32_t CurrentRPM)
{
  // 20200205 KH: process for no tubing status; avoid can't increase pressure.
  //SensorP = curr_mmH2O;
  if ((CurrentRPM > 4000) && (SensorP <= 0))
  {
    SensorP = 0.1;
  }

  uint32_t TargetRPM = GetTargetRPMByFansLaw(SensorP, TargetP, CurrentRPM);

  // Usually, the algorithm will maintain pressure by increasing or decreasing blower RPM.
  // When the I2C of the pressure sensor is abnormal, the system will provide the previous pressure measurement to the algorithm.
  // Because the target pressure is never reached, the algorithm will increase more RPM of the blower.
  // We should avoid such bad user experience by setting RPM from the pressure in screen.
  // limit the amount of change in each RPM. (add by KH)

  int DeltaRPM = TargetRPM - CurrentRPM;

  if (DeltaRPM >= 3000)
  {
    TargetRPM = CurrentRPM + 3000;
  }
  else if (DeltaRPM <= -3000)
  {
    TargetRPM = CurrentRPM - 3000;
  }
  
  // limiting the minimun and maximum RPM output. (add by KH)
  // 3 cmH2O (depend on FOC parameter)
  // if the minimum RPM is smaller than 8000, it will affect the calibration at 3 cmH2O
  if (TargetRPM < 5000)
    TargetRPM = 5000;

  // 29 cmH2O: @therapy pressure is 20cmH2O and Leakage @100LPM
  if (TargetRPM > 40000) // 25500
    TargetRPM = 40000;

  //*RPM_Output = TargetRPM;

  //SetBlowerRPM(TargetRPM, 0);
  //TargetRPM = 40000;
  motorctrl_set_motor_speed(TargetRPM, 10);
}

void PressureCtrl(float outPressure)
{
  uint32_t CurrentRPM = motorctrl_get_motor_speed();

  SetRPMByFansLaw(curr_mmH2O, outPressure, CurrentRPM);
}