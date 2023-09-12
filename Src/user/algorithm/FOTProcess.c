#include "FOTProcess.h"

void AdaptiveHPF(float Input[], int length)
{
    float w = 0.0f;
    float fc = 3.0f;
    float u = fc * 3.14157f / 250.0f;

    for (int n=0 ; n<length ; n++)
    {
        Input[n] = Input[n] - w;
        w = w + 2.0f * u * Input[n];
    }
}

void RemoveDC(float data[], int length)
{
    int i;

    float total = 0;

    for (i=0 ; i<length ; i++)
        total += data[i];
    
    float mean = total / (float)(length);

    for (i = 0; i < length; i++)
        data[i] = data[i] - mean;
}

int GetPhaseDelay(float P[], float Q[], int length)
{
    float maxvalue = 0;
    float crossvalue = 0;
    int phase = 0;

    RemoveDC(P, length);
    RemoveDC(Q, length);

    AdaptiveHPF(Q, length);

    for (int i=0 ; i<20 ; i++)
    {
        crossvalue = 0;

        for (int j=0 ; j<length-i ; j++)
            crossvalue += (P[j]) * (Q[j + i]);

        crossvalue = crossvalue / (float)(length - i);

        if (crossvalue >= maxvalue)
        {
            maxvalue = crossvalue;
            phase = i;
        }
    }
    
    return phase;
}