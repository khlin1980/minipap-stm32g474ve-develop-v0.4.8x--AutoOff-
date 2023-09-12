#include <stdint.h>
#include <math.h>
#include "parameter.h"
#include <string.h>

//static const float gain_factor = 10;
uint16_t SafeTemplate = 160/10;//gain_factor; // 400;800
uint16_t BreathTemplate;
uint16_t tempBreathTemplate;
uint16_t LocalMax;
uint8_t CNT_SAMPLE;
uint16_t TrainingTemplate[8];
uint16_t UpdatingTemplate[10];
uint16_t SamplingTemplate[8]; // Hank 20170510 �ʺA�ҪO���ˮɤ]�������o�i���ҪO�p��ϥ�
uint8_t SUB_NUM;
// uint8_t Template_Update_Pending;
uint8_t OVERESTIMATE;
uint16_t MAX_OVERSTIMATE;
uint8_t TEMPLATE_UPDATE_PENDING;
uint8_t Template_State;
uint16_t TreatmentDuration;

// static uint16_t NormalBreathCompare;
//  threshold:

uint16_t MedianFilter(uint16_t *TemplateArr, int size)
{
    uint16_t SumTemplate = 0;
    uint16_t MaxTemplate = 0;
    uint16_t MinTemplate = 4096;

    for (int i = 0; i < size; i++)
    {
        SumTemplate += TemplateArr[i]; // + MeanTemplate;

        // if (MaxTemplate < TemplateArr[i])
        if (TemplateArr[i] > MaxTemplate)
            MaxTemplate = TemplateArr[i];

        // if (MinTemplate > TemplateArr[i])
        if (TemplateArr[i] < MinTemplate)
            MinTemplate = TemplateArr[i];
    }

    uint16_t MeanTemplate = (SumTemplate - MaxTemplate - MinTemplate) / (size - 2);

    return MeanTemplate;
}

void DynamicTemplateInit()
{
    BreathTemplate = 0;
    tempBreathTemplate = 0;
    LocalMax = 0;
    CNT_SAMPLE = 0;
    TreatmentDuration = 0;

    memset(TrainingTemplate, 0, sizeof(TrainingTemplate));

    memset(UpdatingTemplate, 0, sizeof(UpdatingTemplate));

    memset(SamplingTemplate, 0, sizeof(SafeTemplate));

    SUB_NUM = 0;
    // NormalBreathCompare = 0;
    // HyponeaBreathCompare = 0;
    // ApneaBreathCompare = 0;
    //  Template_Update_Pending = 0;
    OVERESTIMATE = 0;
    MAX_OVERSTIMATE = 0;
    TEMPLATE_UPDATE_PENDING = 0;

    // BreathPhase = NoBreathPhase;
    //  BreathPhaseSwing = NoBreathPhase; // XT50
    // PV_State = PV_State_InspirWaiting;
    Template_State = Template_State_Init;
}
// 1.Initial Stage
// 前100秒，先使用預設的template(目前為250 adjustable)
// 2.Training Stage
// 100秒後 從每8個呼吸波 取出振幅最大值當作模板後
// 重複8次

// 將8個循環取得的呼吸震幅先做中位濾波後平均得到呼吸模板
// 3.Update Stage
// 呼吸穩定後 從每8個呼吸波 取出振幅最大值當做模板後 重複10次更新一次模板
// 若持續100次未更新，則重新擷取模板，回到retraining stage
// #define TrainingNum 4
// #define UpdatingNum 5
// void DynamicTemplate(void)
// void DynamicTemplate(struct Systems *system)
void DynamicTemplateV3(uint16_t BreathAmplitude, uint8_t BreathPhase)
{
    int TrainingNum = 4;
    int UpdatingNum = 5;

    TreatmentDuration++;

    if (TreatmentDuration >= 10000)
        TreatmentDuration = 10000;

    if (BreathPhase != CrossUpPhase)
        return;

    uint16_t NormalBreathCompare = (uint16_t)(BreathTemplate * 0.6);

    // Calculate Template at the end of breath
    switch (Template_State)
    {
    case Template_State_Init: // 1.Initial Stage
        BreathTemplate = SafeTemplate;
        Template_State = Template_State_WaitTraining;
        break;
    case Template_State_WaitTraining:
        if (TreatmentDuration >= 10000) // Could change to treatment timer
            Template_State = Template_State_Training;
        break;
    case Template_State_Training: // 2.Training Stage
                                  // if(BreathAmplitude>320){//fixed by Hank 20161129
        if (LocalMax < BreathAmplitude && BreathAmplitude > SafeTemplate * 0.6)
        {
            // LocalMax = BreathAmplitude;
            SamplingTemplate[CNT_SAMPLE] = BreathAmplitude; // Hank 20170510
        }
        else if (BreathAmplitude < SafeTemplate * 0.6)
        {
            // LocalMax = SafeTemplate*0.8;
            SamplingTemplate[CNT_SAMPLE] = (uint16_t)(SafeTemplate * 0.6); // Hank 20170510
        }

        CNT_SAMPLE = CNT_SAMPLE + 1;

        if (CNT_SAMPLE == 6)
        {
            LocalMax = MedianFilter(SamplingTemplate, 6); // Hank 20170510
            TrainingTemplate[SUB_NUM] = LocalMax;
            LocalMax = 0;
            SUB_NUM++;
            CNT_SAMPLE = 0;
            if (SUB_NUM == TrainingNum)
            { // Hank changing num for dynamic template
                BreathTemplate = MedianFilter(TrainingTemplate, TrainingNum);
                Template_State = Template_State_Update; // Tranisit state to update state
                                                        // SUB_NUM=0;

                for (int i = 0; i < TrainingNum; i++)
                {
                    UpdatingTemplate[i] = TrainingTemplate[i];
                }
                // UpdatingTemplate[SUB_NUM] = BreathTemplate;
                // SUB_NUM++;//SUB_NUM=9;//Hank remove 20200421
            }
        }
        //}
        break;
    case Template_State_Update: // 3.Update Stage
        if (BreathAmplitude > NormalBreathCompare)
        {
            TEMPLATE_UPDATE_PENDING = 0;
            /*
            if(LocalMax < BreathAmplitude)
                LocalMax = BreathAmplitude;
            */
            SamplingTemplate[CNT_SAMPLE] = BreathAmplitude; // Hank 20170510
            CNT_SAMPLE = CNT_SAMPLE + 1;
            if (CNT_SAMPLE == 6)
            {
                LocalMax = MedianFilter(SamplingTemplate, 6); // Hank 20170510
                UpdatingTemplate[SUB_NUM] = LocalMax;
                SUB_NUM++;
                CNT_SAMPLE = 0;
                if (BreathTemplate > LocalMax)
                {

                    OVERESTIMATE++;

                    if (MAX_OVERSTIMATE < LocalMax)
                        MAX_OVERSTIMATE = LocalMax;

                    if (OVERESTIMATE == 8)
                    {
                        BreathTemplate = MAX_OVERSTIMATE;
                        OVERESTIMATE = 0;
                        MAX_OVERSTIMATE = 0;
                    }
                }
                else
                {
                    OVERESTIMATE = 0;
                    MAX_OVERSTIMATE = 0;
                }
                LocalMax = 0;
                tempBreathTemplate = MedianFilter(UpdatingTemplate, UpdatingNum);
                BreathTemplate = (BreathTemplate + tempBreathTemplate) >> 1;
                if (SUB_NUM == UpdatingNum)
                {
                    SUB_NUM = 0;
                }
            } // if (CNT_SAMPLE == 8)
        }
        else
        { //<80%//if (BreathAmplitude > NormalBreathCompare)
            TEMPLATE_UPDATE_PENDING++;
            if (TEMPLATE_UPDATE_PENDING >= 50)
            { // 35){//XT50
                Template_State = Template_State_ReTraining;
                BreathTemplate = SafeTemplate;
                TEMPLATE_UPDATE_PENDING = 0;
                CNT_SAMPLE = 0;
                SUB_NUM = 0;
                LocalMax = 0;
            }
        } // if (BreathAmplitude > NormalBreathCompare)

        break;
    case Template_State_ReTraining:
        if (LocalMax < BreathAmplitude && BreathAmplitude > SafeTemplate * 0.6)
            LocalMax = BreathAmplitude;
        else if (LocalMax < SafeTemplate * 0.6)
            LocalMax = (uint16_t)(SafeTemplate * 0.6);

        CNT_SAMPLE = CNT_SAMPLE + 1;

        if (CNT_SAMPLE == 6)
        {
            TrainingTemplate[SUB_NUM] = LocalMax;
            LocalMax = 0;
            SUB_NUM++;
            CNT_SAMPLE = 0;
            if (SUB_NUM == TrainingNum)
            {
                BreathTemplate = MedianFilter(TrainingTemplate, TrainingNum);
                Template_State = Template_State_Update; // Tranisit state to update state
                                                        // SUB_NUM=0;
                for (int i = 0; i < TrainingNum; i++)
                {
                    UpdatingTemplate[i] = TrainingTemplate[i];
                }
                // UpdatingTemplate[SUB_NUM] = BreathTemplate;
                // SUB_NUM++; //SUB_NUM=9;
            }
        }
        break;

    default:
        break;
    } // switch(Template_State)
    // Calculate breath events threshold
    // Threshold_Update(BreathTemplate, system);
    // Threshold_UpdateV2(BreathTemplate);

    // NormalBreathCompare = (uint16_t)(BreathTemplate * 0.6);
}