// #include "stm32f4xx_hal.h"
// #include "main.h"
#include <stdint.h>
//#include "EventDetection.h"
#include "AutoMode.h"
#include "parameter.h"

enum Automode_State
{
    Automode_State_WaitBreathStatus,       // 0
    Automode_State_OSAAscendReaction,      // 1
    Automode_State_CSAAscendReaction,      // 2
    Automode_State_HypopneaAscendReaction, // 3
    Automode_State_DeclinePressure,        // 4
    Automode_State_FLAscendReaction        // 5
};

void iXAUTO_AUTOMODE_CDC_Trace(uint8_t P1, uint8_t P2)
{
#if 0
    char buffer[256];
    //sprintf(buf,"\r\n");
    //strcat(CDC_Buffer, buf);
    //sprintf(CDC_Buffer, "Cnt:%d Pres:%d Flow:%d Basel:%d Amp:%d H_Thre:%d A_Thre:%d R_Basel:%d A_diff:%d A_dur:%d\ Phase:%d\r\n",CDC_Cnt,autocount,FlowLPM_M0,BreathBaseline,BreathAmplitude,HyponeaBreathCompare,ApneaBreathCompare,RoughBaseline,ApneaDiff,ApneaDuration,BreathPhase);
    //sprintf(CDC_Buffer, "R_base:%d A_diff:%d Flow:%d is_A:%d Amp:%d H_Thre:%d A_Thre:%d A_Count:%d H_Count:%d AHI_stat:%d\r\n",RoughBaseline,ApneaDiff,FlowLPM_M0,isApnea,BreathAmplitude,HyponeaBreathCompare,ApneaBreathCompare,AHI_info.Count_Apnea,AHI_info.Count_Hypopnea,AHI_DECT_S);
    //sprintf(CDC_Buffer, "Cnt:%d Pres:%d Flow:%d R_Basel:%d Amp:%d A_thre:%d H_thre:%d \r\n",CDC_Cnt,TargetPressure,FlowLPM_M0,RoughBaseline,BreathAmplitude,ApneaBreathCompare,HyponeaBreathCompare);
    //sprintf(CDC_Buffer, "Flow:%d Amp:%d H_Thre:%d A_Thre:%d A_diff:%d Bline:%d AI:%d HI:%d TargetP:%d A_C:%d H_C:%d PVA_S:%d\r\n",PQ_Curve.FlowLPM_M0,BreathAmplitude,HyponeaBreathCompare,ApneaBreathCompare,ApneaDiff,BreathBaseline,AHI_info.ApneaIndex,AHI_info.HyponeaIndex,(int)FANLAW.Mask_P,AHI_info.Count_Apnea,AHI_info.Count_Hypopnea,PVA_Status);
    //sprintf(CDC_Buffer, "Flow:%d Amp:%d H_Thre:%d A_Thre:%d Mic_TargetP:%d Bline:%d TargetP:%d A_C:%d H_C:%d PVA_S:%d Breath_S:%d\r\n",PQ_Curve.FlowLPM_M0,BreathAmplitude,HyponeaBreathCompare,ApneaBreathCompare,TargetPressure,BreathBaseline,(int)FANLAW.Mask_P,AHI_info.Count_Apnea,AHI_info.Count_Hypopnea,PVA_Status,BreathPhase);
    //sprintf(CDC_Buffer,"nLeak:%d TargetP:%d MaskP:%d\r\n",(int)cLeakFlow,(int)FANLAW.TP,(int)FANLAW.Mask_P);
    //sprintf(CDC_Buffer, "No:%d Flow:%d \r\n",CDC_Cnt,PQ_Curve.FlowLPM_M0);phase_delay
    //	sprintf(CDC_Buffer, "Flow:%d Amp:%d H_Thre:%d TargetP:%d UnInten_Leak:%d A_C:%d H_C:%d AI:%d HI:%d SI:%d CI:%d AHI_S:%d Breath_S:%d\r\n",PQ_Curve.FlowLPM_M0,BreathAmplitude,HyponeaBreathCompare,(int)system->TargetP,MaskFlow,AHI_info.Count_Apnea,AHI_info.Count_Hypopnea,AHI_info.ApneaIndex,AHI_info.HyponeaIndex,AHI_info.SnoreIndex,AHI_info.CentralApneaIndex,AHI_DECT_S,BreathStatus);
    sprintf(buffer, "%d\r\n%d\r\n", P1,P2);

    CDC_Transmit_FS((uint8_t *)buffer, strlen(buffer)); //AUTO_S->MaskFlow
#endif
}

uint8_t AUTO_S; // = Automode_State_WaitBreathStatus;

int StateCounter_Normal = 0;
int StateCounter_Apnea = 0;
int StateCounter_Hypopnea = 0;

int ResponseTime_Apnea = 0;
int ResponseTime_Hypopnea = 0;

int ApneaAscendPressureTimes = 0;

int PressureAscend05Counter = 0;

// int A;
// int F;
static int SampleRate_Count = 100;

float max_pressure_setting = 200;
float min_pressure_setting = 40;

uint8_t Boosting = 0;
uint8_t PrevBreathStatus = 0;
uint8_t PrepareAscendPressure = 0;
uint8_t UpdateRecordTargetP = 0;

float PrevTargetPressure = 0;
uint8_t DescendFlag = 0;

//extern TypeAHI_info AHI_info;

void AutoModeTaskInit(float minP, float maxP, int SamplePerSecond)
{
    max_pressure_setting = maxP;
    min_pressure_setting = minP;
    SampleRate_Count = SamplePerSecond;
    AUTO_S = Automode_State_WaitBreathStatus;
    Boosting = 0;
    UpdateRecordTargetP = 0;
    DescendFlag = 0;//AHI_info.DescendFlag = 0;
    
}

/*
float PressureCompensation()
{
    if(AHI_info.DescendFlag)
    {
        AHI_info.DescendFlag = 0;//
        return 2; //0.2cmH2O
    }

    return 0;
}
*/

void AutoModeTaskProcess(uint8_t BreathStatus, float *TargetPressure, float *RecordTargetPressure)
{
    if (Boosting)
        BreathStatus = PrevBreathStatus;

    switch (AUTO_S)
    {
    case Automode_State_WaitBreathStatus:
        // PR032-12h
        if (DescendFlag)//(AHI_info.DescendFlag)
        {
            DescendFlag = 0;//AHI_info.DescendFlag = 0;
            *TargetPressure = PrevTargetPressure;
            //*RecordTargetPressure = PrevTargetPressure;
        }

        // PR032-12h
        PrepareAscendPressure = 0;

        switch (BreathStatus)
        {
        case BreathStatus_OSA:
            ResponseTime_Apnea = 0;
            StateCounter_Apnea = 0;
            PressureAscend05Counter = 0;
            ApneaAscendPressureTimes = 0;
            AUTO_S = Automode_State_OSAAscendReaction;
            // PR032-12h
            PrepareAscendPressure = 5;
            break;

        case BreathStatus_CSA:
            AUTO_S = Automode_State_CSAAscendReaction;
            break;

        case BreathStatus_Hypopnea:
            ResponseTime_Hypopnea = 0;
            StateCounter_Hypopnea = 0;
            PressureAscend05Counter = 0;
            AUTO_S = Automode_State_HypopneaAscendReaction;
            // PR032-12h
            PrepareAscendPressure = 5;
            break;

        case BreathStatus_FlowLimitation:
            AUTO_S = Automode_State_FLAscendReaction;
            // PR032-12h
            PrepareAscendPressure = 2;
            break;

        case BreathStatus_NormalBreath:
            StateCounter_Normal = 0;
            AUTO_S = Automode_State_DeclinePressure;
            break;

        default:
            break;
        }

        // PR032-12h
        *RecordTargetPressure = *TargetPressure + PrepareAscendPressure;
        UpdateRecordTargetP = 0; // not update the RecordTargetPressure

        /*PR032-12d
        if(AUTO_S != Automode_State_DeclinePressure)
        {
            *TargetPressure += PressureCompensation();
        }
        */
        break;

    case Automode_State_OSAAscendReaction: // OSA event occurs

        StateCounter_Apnea++;
        // Add to keep timeout to prevent abnormal situation /modify in 20170324
        Boosting = (StateCounter_Apnea >= (ResponseTime_Apnea * SampleRate_Count) && ApneaAscendPressureTimes < 3) ? 1 : 0;

        if (Boosting)
        {
            PrevBreathStatus = BreathStatus_OSA;

            PressureAscend05Counter++;

            if (PressureAscend05Counter % 20 == 0)
                *TargetPressure += 1;

            if (PressureAscend05Counter == 100)
            {
                // Reset:
                PressureAscend05Counter = 0;
                StateCounter_Apnea = 0;

                // Set response time to 25 sec.
                ResponseTime_Apnea = 25;

                ApneaAscendPressureTimes++;

                Boosting = 0;
            }
        }

        if (!Boosting)
        {
            if (BreathStatus != BreathStatus_OSA)
                AUTO_S = Automode_State_WaitBreathStatus;
        }

        break;

    case Automode_State_HypopneaAscendReaction: // Hypopnea event occurs
        StateCounter_Hypopnea++;
        // Add to keep timeout to prevent abnormal situation /modify in 20170324
        Boosting = (StateCounter_Hypopnea >= (ResponseTime_Hypopnea * SampleRate_Count)) ? 1 : 0;

        if (Boosting)
        {
            PrevBreathStatus = BreathStatus_Hypopnea;

            PressureAscend05Counter++;

            if (PressureAscend05Counter % 20 == 0)
                *TargetPressure = *TargetPressure + 1;

            if (PressureAscend05Counter == 100)
            {
                // Reset:
                PressureAscend05Counter = 0;
                StateCounter_Hypopnea = 0;

                // Set:
                ResponseTime_Hypopnea = (*TargetPressure >= 100) ? 60 : 45;

                Boosting = 0;
            }
        }

        if (!Boosting)
        {
            if (BreathStatus != BreathStatus_Hypopnea)
                AUTO_S = Automode_State_WaitBreathStatus;
        }

        break;

    case Automode_State_DeclinePressure: // Normal

        StateCounter_Normal++;

        if (StateCounter_Normal == 60 * SampleRate_Count)
        {
            PrevTargetPressure = *TargetPressure;
            *TargetPressure = *TargetPressure - 2;
            StateCounter_Normal = 0;

            DescendFlag = 1;//AHI_info.DescendFlag = 1; // occurs descending pressure.
        }

        if (BreathStatus != BreathStatus_NormalBreath)
        {
            AUTO_S = Automode_State_WaitBreathStatus;
            // PR032-12e
            /*
            if(AHI_info.DescendFlag)
            {
                AHI_info.DescendFlag = 0;
                *TargetPressure = PrevTargetPressure;
            }
            */
            //*TargetPressure += PressureCompensation();
        }

        break;

    case Automode_State_CSAAscendReaction: // CSA event occurs

        if (BreathStatus != BreathStatus_CSA)
            AUTO_S = Automode_State_WaitBreathStatus;

        break;

    case Automode_State_FLAscendReaction: // Flow limitation event occurs

        *TargetPressure = *TargetPressure + 2;

        if (BreathStatus != BreathStatus_FlowLimitation)
            AUTO_S = Automode_State_WaitBreathStatus;

        break;

    default:
        break;
    }

    if (*TargetPressure < min_pressure_setting)
    {
        *TargetPressure = min_pressure_setting;
    }
    else if (*TargetPressure > max_pressure_setting)
    {
        *TargetPressure = max_pressure_setting;
    }

    if (Boosting == 0 && UpdateRecordTargetP)
    {
        *RecordTargetPressure = *TargetPressure;
    }

    // add protection of recording pressure:
    if (*RecordTargetPressure < min_pressure_setting)
    {
        *RecordTargetPressure = min_pressure_setting;
    }
    else if (*RecordTargetPressure > max_pressure_setting)
    {
        *RecordTargetPressure = max_pressure_setting;
    }

    UpdateRecordTargetP = 1;

    // iXAUTO_AUTOMODE_CDC_Trace((int)*TargetPressure, (int)*RecordTargetPressure);
}
