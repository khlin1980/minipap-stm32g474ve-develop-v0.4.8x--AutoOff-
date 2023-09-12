#include "LeakManagement.h"
#include "math.h"

float ZeroLeakQ[3] = {0, 0, 0};
float ZeroLeakP[3] = {30, 100, 200};
/***********************************************************
FUNCTION: LeakManagementInit
Decription:  Initialize ZeroLeakBaseline
Argument: N/A
Return:  N/A
Note:
***********************************************************/

//void LeakManagementInit(unsigned int ZeroLeakBaseline[3])
void LeakManagementInit(uint16_t ZeroLeakBaseline[3])
{
    for (int i = 0; i < 3; i++)
        ZeroLeakQ[i] = (float)(ZeroLeakBaseline[i]);
}

float Interpolation(float x0, float x1, float y0, float y1, float x)
{
    //(y-y0)/(x-x0) = (y1-y0)/(x1-x0)
    // float ZeroLeakBaseline = ((Pressure - ZeroLeakP[index0]) * (ZeroLeakQ[index1] - ZeroLeakQ[index0]) / (ZeroLeakP[index1] - ZeroLeakP[0])) + ZeroLeakBL[0];
    return (x - x0) * (y1 - y0) / (x1 - x0) + y0;
}
/***********************************************************
FUNCTION: GetZeroLeakFlow
Decription: Get ZeroLeakFlow
Argument: N/A
Return:  ZeroLeakFlow
Note:
***********************************************************/

float GetZeroLeakFlow(float FinalP)
{
    float ZeroLeakFlow = 0;

    if (FinalP <= ZeroLeakP[1])
    {
        ZeroLeakFlow = Interpolation(ZeroLeakP[0], ZeroLeakP[1], ZeroLeakQ[0], ZeroLeakQ[1], FinalP);
    }
    else
    {
        ZeroLeakFlow = Interpolation(ZeroLeakP[1], ZeroLeakP[2], ZeroLeakQ[1], ZeroLeakQ[2], FinalP);
    }

    return ZeroLeakFlow;
}

// float nLeakFlow = 0;
/***********************************************************
FUNCTION: EstimateLeakQ
Decription: calulation cLeakFlow
Argument: N/A
Return:  cLeakFlow
Note:
***********************************************************/
float nLeakFlow;
float ZeroLeakFlow;
float cLeakFlow;
uint16_t EstimateLeakQ(float Flow, float TargetP, float FinalP, uint8_t withHumidifier)
{
    //float nLeakFlow = 0;
    //float cLeakFlow = 0;

     ZeroLeakFlow = GetZeroLeakFlow(FinalP);
    // float ZeroLeakFlow = GetZeroLeakFlow(FinalP);

    if (Flow >= ZeroLeakFlow + 10) // 0LPM
        nLeakFlow = Flow - ZeroLeakFlow;

    if (nLeakFlow == 0)
        return 0;

    float EquationCoefficient[2][6][3] = {{
                                              // Without Humidifier
                                              {0.000009f, 0.1122f, 0},      // TP < 80
                                              {0, 0.0904f, 14.67f},         // TP < 80
                                              {-0.00002f, 0.1246f, 0},      // TP < 200
                                              {0.00005f, 0.0894f, 0},       // TP = 200
                                              {-0.00003f, 0.1303f, 0.1476f} // TP = 200
                                          },
                                          {
                                              // With Humidifier
                                              {0.00002f, 0.1123f, 0},   // TP < 80
                                              {0, 0.0928f, 15.063f},    // TP < 80
                                              {-0.000006f, 0.1201f, 0}, // TP < 200
                                              {0, 0.0836f, 19.586f},    // TP < 200
                                              {0.00005f, 0.0904f, 0},   // TP = 200
                                              {0, 0.089f, 15.319f}      // TP = 200
                                          }};

    uint8_t EquationIndex = 0;

    // MaskFlow = 0.1038 * (unintential_LeakFlow); //y=0.1287x

    if (!withHumidifier)
    {
        if (TargetP < 80)
        {
            EquationIndex = (nLeakFlow <= 60) ? 0 : 1;
        }
        else if (TargetP < 200)
        {
            EquationIndex = 2;
        }
        else if (TargetP >= 200)
        {
            EquationIndex = (nLeakFlow <= 50) ? 3 : 4;
        }
    }
    else
    {
        if (TargetP < 80)
        {
            EquationIndex = (nLeakFlow <= 52) ? 0 : 1;
        }
        else if (TargetP < 200)
        {
            EquationIndex = (nLeakFlow <= 70) ? 2 : 3;
        }
        else if (TargetP >= 200)
        {
            EquationIndex = (nLeakFlow <= 50) ? 4 : 5;
        }
    }

    // y = ax^2 + bx + c
    for (int i = 0; i < 3; i++)
        cLeakFlow += EquationCoefficient[withHumidifier][EquationIndex][i] * (float)(pow(nLeakFlow, 2 - i));

    // MaskFlow = (uint16_t)cLeakFlow;
    //return (uint16_t)(cLeakFlow+0.5f);
    
    return nLeakFlow;
}

/***********************************************************
FUNCTION: GetMaxLeakP
Decription: calulation MaxLeakP
Argument: N/A
Return:  MaxLeakP
Note: avoid LeakP compensate over
***********************************************************/
float GetMaxLeakP(float TargetP, float LeakQ, uint8_t withHumidifier)
{
    float MaxP[2][3][10] = {{
                                // without Humidifier
                                {0, 1, 3, 7, 11, 16, 19, 26, 33, 37}, // TP < 80
                                {0, 2, 5, 9, 14, 19, 23, 30, 36, 41}, // TP < 200
                                {0, 2, 6, 11, 16, 22, 27, 34, 43, 46} // TP = 200
                            },
                            {
                                // with Humidifier
                                {0, 2, 5, 10, 16, 23, 28, 37, 46, 52}, // TP < 80
                                {0, 3, 7, 12, 18, 26, 34, 44, 53, 64}, // TP < 200
                                {0, 5, 10, 16, 4, 33, 39, 49, 60, 68}  // TP = 200
                            }};

    float Leak[11] = {0, 5, 15, 25, 35, 45, 55, 65, 75, 85, 200};

    float MaxLeakP = 0;

    uint8_t indexP = 0;

    if (TargetP < 80)
    {
        indexP = 0;
    }
    else if (TargetP < 200)
    {
        indexP = 1;
    }
    else if (TargetP >= 200)
    {
        indexP = 2;
    }

    for (int i = 0; i < 10; i++)
    {
        if (LeakQ > Leak[i] && LeakQ <= Leak[i + 1])
        {
            MaxLeakP = MaxP[withHumidifier][indexP][i];
            break;
        }
    }

    return MaxLeakP;
}

/***********************************************************
FUNCTION: EstimateLeakP
Decription: calulation LeakP
Argument: N/A
Return:  EstimateLeakP
Note:
***********************************************************/
float EstimateLeakP(float LeakQ, float TargetP, uint8_t withHumidifier)
{
    float EquationCoefficient[2][4][3] = {{
                                              // without Humidifier
                                              {0.0037f, 0.1146f, 0}, // TP<80
                                              {0.0034f, 0.2006f, 0}, // TP<200
                                              {0.004f, 0.2358f, 0}   // TP=200
                                          },
                                          {
                                              // with Humidifier
                                              {0.0047f, 0.1991f, 0}, // TP<80
                                              {0.0055f, 0.2198f, 0}, // TP<160
                                              {0.0059f, 0.252f, 0},  // TP<200
                                              {0.0054f, 0.3152f, 0}  // TP=200
                                          }};
    float LeakP = 0;
    float LeakFactor = 0;

    uint8_t EquationIndex = 0;

    // Limit Leak Q to 100LPM
    if (LeakQ >= 100)
        LeakQ = 100;

    if (!withHumidifier) // Without Humidifier
    {
        if (TargetP < 80)
        {
            EquationIndex = 0;
            LeakFactor = 0.9f;
        }
        else if (TargetP < 200)
        {
            EquationIndex = 1;
            LeakFactor = 0.925f;
        }
        else if (TargetP >= 200)
        {
            EquationIndex = 2;
            LeakFactor = (LeakQ <= 65) ? 0.85f : 0.875f;
        }
    }
    else
    {
        if (TargetP < 80)
        {
            EquationIndex = 0;
            LeakFactor = 0.96f;
        }
        else if (TargetP < 160)
        {
            EquationIndex = 1;
            LeakFactor = (LeakQ < 70) ? 0.95f : 0.9f;
        }
        else if (TargetP < 200)
        {
            EquationIndex = 2;
            LeakFactor = (LeakQ < 70) ? 0.95f : 0.9f;
        }
        else if (TargetP >= 200)
        {
            EquationIndex = 3;
            LeakFactor = 0.925f;
        }
    }

    // y = ax^2 + bx + c
    for (int i = 0; i < 3; i++)
        LeakP += EquationCoefficient[withHumidifier][EquationIndex][i] * (float)(pow(LeakQ, 2 - i));

    LeakP = LeakP * LeakFactor;

    if (LeakQ <= 85)
    {
        float MaxLeakP = GetMaxLeakP(TargetP, LeakQ, withHumidifier);

        if (LeakP > MaxLeakP)
            LeakP = MaxLeakP;
    }

    return LeakP;
}
/***********************************************************
FUNCTION: LeakManagementTask
Decription: LeakManagementTask
Argument: N/A
Return:
Note:
***********************************************************/
void LeakManagementTask(float Flow, float TargetP, float FinalP, uint8_t withHumidifier, uint16_t *LeakQ, float *LeakP)
{
    // step1 : estimate unintentional leak:
    *LeakQ = EstimateLeakQ(Flow, TargetP, FinalP, withHumidifier);

    // step2 : estimate the compensation pressure for unintentional leak:
    *LeakP = EstimateLeakP(*LeakQ, TargetP, withHumidifier);
}