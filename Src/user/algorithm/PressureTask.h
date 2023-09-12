#ifndef __PressureTask_H
#define __PressureTask_H

void PressureTaskInit(struct Systems *system);
//void PressureTaskOfRunMaskFit();
//void PressureTaskOfCalibration();
//void PressureTaskProcessOfCalibration(struct Systems *system);

//void PressureTaskOfTreatment(struct Systems *system);
float PressureTaskProcessOfTreatment(struct Systems *system);

#endif
