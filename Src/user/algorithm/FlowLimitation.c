#include <stdint.h>
#include <math.h>
#include "FlowLimitation.h"
// float view[255]=0;
float RecognizeFlowLimitation(float InspiratoryFlow[], uint8_t FL_NUM)
{
    if (FL_NUM > 225 || FL_NUM < 25)
        return 0;

    float Offset = InspiratoryFlow[0];
    float PeakFlow = 0.1f;

    float Weight[225];

    float MatrixA[4][8] = {0};
    float MatrixB[4] = {0};

    float Coefficients[4] = {0};

    for (int i = 0; i < FL_NUM; i++)
    {
        InspiratoryFlow[i] -= Offset;

        if (InspiratoryFlow[i] >= PeakFlow)
            PeakFlow = InspiratoryFlow[i];
    }

    for (int i = 0; i < FL_NUM; i++)
    {
        InspiratoryFlow[i] = InspiratoryFlow[i] / PeakFlow;
        // view[i] = InspiratoryFlow[i];
    }

    // 權重
    for (int i = 0; i < FL_NUM; i++)
        Weight[i] = 1;

    Weight[0] = Weight[FL_NUM - 1] = 100;
    Weight[(uint8_t)(FL_NUM / 2)] = 500;

    // FLFlowI[i] = FLFlow[i] / BreathAmplitude; //BreathAmplitude Normalization

    // Building Matrix A
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            // float ArrayValue = 0;
            MatrixA[i][j] = 0;

            for (int k = 0; k < FL_NUM; k++)
                MatrixA[i][j] += pow(k + 1, i + j) * Weight[k];

            // ArrayValue = ArrayValue + pow(k + 1, i + j) * Weight[k];

            // MatrixA[i][j] = ArrayValue;
        }
    }

    // Building Matrix B
    for (int i = 0; i < 4; i++)
    {
        // float Array_Value = 0;
        MatrixB[i] = 0;

        for (int j = 0; j < FL_NUM; j++)
        {
            MatrixB[i] += (pow(j + 1, i) * InspiratoryFlow[j] * Weight[j]);
            // Array_Value = Array_Value + pow(j + 1, i) * InspiratoryFlow[j] * Weight[j];
        }
        // MatrixB[i] = Array_Value;
    }

    // Building Expansion matrix of Matrix A [A|I]
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            MatrixA[i][4 + j] = 0;
        }
    }

    for (int i = 0; i < 4; i++)
    {
        MatrixA[i][4 + i] = 1;
    }

    // Check Matrix A determinant =0
    // Determinant = 0;
    float Determinant = 0;

    for (int i = 0; i < 4; i++)
    {
        Determinant = Determinant + MatrixA[0][(0 + i) % 4] * MatrixA[1][(1 + i) % 4] * MatrixA[2][(2 + i) % 4] * MatrixA[3][(3 + i) % 4];
        Determinant = Determinant - MatrixA[3][(0 + i) % 4] * MatrixA[2][(1 + i) % 4] * MatrixA[1][(2 + i) % 4] * MatrixA[0][(3 + i) % 4];
    } // TODO if determinant is 0, no inverse matrix. Do not find the inverse matrix

    if (Determinant >= -0.000001 && Determinant <= 0.000001)
        return 0;

    // if Determinant value close to 0, disable to make inverse matrix
    // Build triangular matrix
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            if (i < j && MatrixA[j][i] != 0)
            { // if position of is not bevel and not 0
                float temp = MatrixA[j][i] / MatrixA[i][i];
                for (int k = 0; k < 8; k++)
                    MatrixA[j][k] = MatrixA[j][k] - MatrixA[i][k] * temp;
            }
        }
    }

    for (int i = 0; i < 4; i++)
    {
        float temp = MatrixA[i][i];
        for (int k = 0; k < 8; k++)
        { // First, let the value of left side bevel turn to 1
            MatrixA[i][k] = MatrixA[i][k] / temp;
        }
    }

    // inverse iteration;to form [I][C] ;inverse matrix of A is C
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            if (i != j && MatrixA[i][j] != 0)
            {
                float temp = MatrixA[i][j] / MatrixA[j][j];
                for (int k = 0; k < 8; k++)
                {
                    MatrixA[i][k] = MatrixA[i][k] - MatrixA[j][k] * temp;
                }
            }
        }
    }

    for (int i = 0; i < 4; i++)
    {
        // Coef_Value = 0;
        Coefficients[i] = 0;

        for (int j = 0; j < 4; j++)
        {
            // Coef_Value = Coef_Value + FL_MatrixA[i][j + 4] * FL_MatrixB[j];
            Coefficients[i] += MatrixA[i][j + 4] * MatrixB[j];
        }
        // Coef_c[i] = Coef_Value; // 第n項為n次方的係數
    }

    // Calculate difference

    float ErrorSum = 0;

    for (int i = 0; i < FL_NUM; i++)
    {
        // FittingCurve[i] = Coef_c[0] + Coef_c[1] * (i + 1) + Coef_c[2] * (i + 1) * (i + 1) + Coef_c[3] * (i + 1) * (i + 1) * (i + 1);
        // float CurveValue = Coefficients[0] + Coefficients[1] * (i + 1) + Coefficients[2] * (i + 1) * (i + 1) + Coefficients[3] * (i + 1) * (i + 1) * (i + 1);
        float CurveValue = 0;

        for (int j = 0; j < 4; j++)
            CurveValue += Coefficients[j] * pow(i + 1, j);

        float error = InspiratoryFlow[i] - CurveValue;

        if (error < 0)
            error = -error;

        ErrorSum += error;

        /*
        if (FLFlowI[i] > FittingCurve[i])
            temp_error = temp_error + FLFlowI[i] - FittingCurve[i];
        else
            temp_error = temp_error + FittingCurve[i] - FLFlowI[i];
        */
    }

    // FL_Error = temp_error / FL_NUM;
    // FLIndex = FL_Error;

    float ErrorMean = ErrorSum / (float)(FL_NUM);

    // temp3 = ErrorMean;

    return ErrorMean;
}