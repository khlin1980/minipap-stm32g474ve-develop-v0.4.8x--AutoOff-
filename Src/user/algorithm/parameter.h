#ifndef __PARAMETER_H
#define __PARAMETER_H

#define GLIST
#define BETA 0 // beta test not for release

#define VERSION_MAJOR 1
#define VERSION_MINOR 1
#define VERSION_REVISION 5
#define VERSION_BUILD 3
#define VERSION_NOTE ""

#define VERSION_NUMERIZATION (VERSION_MAJOR * 10000000lu + VERSION_MINOR * 100000lu + VERSION_REVISION * 1000lu + VERSION_BUILD)
#define DeviceTypeID 209

// Version for Graphic Resource
#define GR_VERSION 105
#define LANGUAGE_NUM 12
#define BMP_NUM 305
#define GR_ID_CODE (uint16_t)(0x5AA5)

#define IMAX(N, M) ((N) > (M) ? (N) : (M))

enum BreathStatusTypes
{
    BreathStatus_NormalBreath,   // 0
    BreathStatus_OSA,            // 1
    BreathStatus_CSA,            // 2
    BreathStatus_Hypopnea,       // 3
    BreathStatus_FlowLimitation, // 4
    BreathStatus_Snore           // 5
};

enum _PV_State
{
    PV_State_InspirWaiting,
    PV_State_PeakFinding,
    PV_State_PeakChecking,
    PV_State_PeakChecked,
    PV_State_ValleyFinding,
    PV_State_ValleyChecking,
    PV_State_ValleyChecked,
    PV_State_IneffectiveBreath
};

enum _Template_State
{
    Template_State_Init,
    Template_State_WaitTraining,
    Template_State_Training,
    Template_State_Update,
    Template_State_ReTraining,
    Template_State_ReInit
};

enum _BaselineCal_State
{
    BaselineCal_State_IDLE,
    BaselineCal_State_Detect,
    BaselineCal_State_Confirm
};

typedef struct
{
    float Dy;
    float NextDy;
    float D2y;
    float NextD2y;
} structDiff;

typedef struct
{
    uint16_t TempPeak_Val;
    uint64_t TempPeak_Pos;
    uint16_t Peak_Val;
    uint64_t Peak_Pos;
    uint16_t TempValley_Val;
    uint64_t TempValley_Pos;
    uint16_t Valley_Val;
    uint64_t Valley_Pos;
    uint64_t Pre_Peak_Pos;
} structPVInfo;

enum _CheckPhase_State
{
    CheckPhase_STATE_IDLE,
    CheckPhase_STATE_START
};

enum _BreathPhase
{
    NoBreathPhase,
    DownPhase,
    CrossUpPhase,
    UpperPhase,
    CrossDownPhase
};

typedef struct
{
    unsigned char seconds;
    unsigned char minutes;
    unsigned char hours;
    unsigned char weekday;
    unsigned char monthday;
    unsigned char month;
    unsigned char year;
} time_format_t;

typedef struct
{
    unsigned int min;      // ramp time
    unsigned int pressure; // ramp pressure
    unsigned int step;     // step rpm
    unsigned int type;     // ramp type
    unsigned int base;     // init rpm for init p
    unsigned int rpm;
} ramp_parameter_t;

typedef struct
{
    unsigned int _1d;
    unsigned int _7d;
    unsigned int _30d;
    unsigned int _90d;
} report_struct_t;

#if 0
typedef struct
{
    unsigned int AvgPress[4];   //1d, 7d, 30d, 90d
    unsigned int _90th[4];
    unsigned int Leakage[4];
    unsigned int AHI[4];
    unsigned int CompTime[4];
    unsigned int AvgTime[4];
} ixauto_report_t;
#endif

typedef struct
{
#if 1
    uint8_t PERIOD;
    uint16_t UDAYS;
    // uint16_t UHOURS;
    float UHOURS;
    float BHOURS;
    float AVGT;
    uint16_t RATE;
    float AHI;
    float TAI;
    float CAI; // 1d, 7d, 30d, 90d
    uint16_t AVGP;
    uint16_t NINETYP;
    float LEAK;
#else
    unsigned int PERIOD;
    unsigned int AHI[4];
    unsigned int TAI[4];
    unsigned int CAI[4]; // 1d, 7d, 30d, 90d
    unsigned int AvgTime[4];
    unsigned int ComRate[4];
    unsigned int AvgPressure[4];
#endif
} iXAUTO_REPORT_T;

#if 0
typedef struct
{
    unsigned char alarm; //enble or disable
    unsigned char autochk;
    unsigned char tube; //15mm, 22mm
    unsigned char filter;
    unsigned char lpa;
    unsigned char unit; //cmh2o, hpa
    unsigned char lang; //0-4
} ixauto_setting_t;
#endif

typedef struct
{
    uint8_t SD;
    uint8_t Wifi;
#ifdef BOARD19
    uint8_t Filter;
    uint8_t HH;
    uint8_t Alarm;
    uint8_t BT;
#endif
} ixauto_status_t;

typedef struct
{
    uint32_t sta; // Start Address of graphic
    uint32_t len; // length of graphic
} iXAUTO_BMP_T;

typedef struct
{
    iXAUTO_BMP_T G[BMP_NUM];
} iXAUTO_GRAPHIC_ARRAY_T;

// iXAUTO SETTING PARATER
typedef struct
{
#if 1
    uint8_t RAMP_T;  // RAMP Time, AUTO/OFF/5-45mins
    uint8_t RAMP_P;  // RAMP Pressure, OFF/4 0.2?
    uint8_t AUTOON;  // Auto ON/OFF
    uint8_t AUTOOFF; // Auto ON/OFF
    uint8_t HH;      // HH Level, 0-6
    uint8_t HT;      // Heated Tube Temperature, OFF; 16-30'C
    uint8_t MASK;    // Mask Type, 3 type
#else
    uint8_t RAMP_T; // RAMP Time, AUTO/OFF/5-45mins
    uint8_t RAMP_P; // RAMP Pressure, OFF/4 0.2?
    uint8_t PVA;    // OFF/ON 0-3
    uint8_t HH;     // HH Level, 0-6
    uint8_t HT;     // Heated Tube Temperature, OFF; 16-30'C
    uint8_t AUTOON; // Auto ON/OFF
    uint8_t MASK;   // Mask Type, 3 type
#endif
} iXAUTO_COMFORT_T;

typedef struct
{
#if 1
    uint8_t FLIGHT;
    uint8_t WiFi;
    uint8_t LTE;
    uint8_t BKL;
    uint8_t ALARM;
    uint8_t ALARMTIME[2];
    uint8_t REMINDER;
    uint8_t LANGUAGE;
    // uint8_t DATE;
    // uint8_t TIME;
    uint8_t UNIT;
#else
    uint8_t ALARM[2];
    uint8_t pUNIT;
    uint8_t tUNIT;
    uint8_t FLIGHT;
    uint8_t WiFi;
    uint8_t LTE; // FOURG;
    uint8_t LPA;
    uint8_t BKL; // 1-10
    uint8_t FILTER;
    uint8_t LANG;
#endif
} iXAUTO_SETTING_T;

// iXAUTO_SETTING_T iXAUTO_SETTING_DEFAULT = {0, 0, 0, 0, 0, 0, 0, 1, 5, 0, 0 };

typedef struct
{
    uint16_t max;
    uint16_t init;
    uint16_t min;
} apap_parameter_t;

typedef struct
{
    unsigned int target;
} cpap_parameter_t;

typedef struct
{
    uint8_t CA; // CPAP APAP
    cpap_parameter_t CPAP;
    apap_parameter_t APAP;
    uint8_t PVA;
} iXAUTO_CLINICAL_T;

// iXAUTO_CLINICAL_T iXAUTO_CLINICAL_DEFAULT = {0, 10, 6, 20, 4, 6 };
/*
typedef struct
{
    unsigned long SID;         //session ID
    unsigned int intBlowHours; //blowing hours, alias intAccProcessHours
    unsigned char chrBlowMins; //blowing mins
    unsigned int intCompHours; //compliance hours, alias intAccCprocessHours
    unsigned char chrCompMins; //compliance mins
    unsigned long LASTID;      //Last time uploaded ID
} iXAUTO_SESSION_T;
*/
typedef struct
{
    unsigned long SID;            // session ID
    unsigned long BlowerMins;     // blowing hours, alias intAccProcessHours
    unsigned long ComplianceMins; // compliance mins
    unsigned long LASTID;         // Last time uploaded ID
} iXAUTO_SESSION_T;

typedef struct
{
    uint8_t ALARM[3]; // YYMMDD
    uint8_t PERIOD;
    uint8_t ON;
} iXAUTO_REMINDER_ALARM_T;

typedef struct
{
    iXAUTO_REMINDER_ALARM_T FILTER;
    iXAUTO_REMINDER_ALARM_T CHAMBER;
    iXAUTO_REMINDER_ALARM_T TUBE;
} iXAUTO_REMINDER_T;

typedef struct
{
    unsigned char hours;
    unsigned char mins;
} iXAUTO_ALARM_T;

typedef struct
{
    unsigned char yy;
    unsigned char mm;
    unsigned char dd;
} iXAUTO_DATE_T;

typedef struct
{
    unsigned char hh;
    unsigned char mm;
    unsigned char ss;
} iXAUTO_TIME_T;

/*
typedef struct
{
    uint8_t StartP;     //Init Pressure
    uint32_t StartRPM;  //Base RPM
    uint8_t TargetP;    //Target Pressure
    uint32_t TargetRPM; //Target RPM
    uint8_t StepP;      //Step Pressure
    uint32_t StepRPM;   //Step RPM
    uint16_t Sec;       //Ramp Time for second
}iXAUTO_RAMP_T;
*/
typedef struct
{
    uint8_t now;
    uint8_t pre;
} iXAUTO_HH_T;

typedef enum
{
    hhNA = 0,
    hhConnect = 1, // Hall sensor detected
    hhChamber = 2,
    hhTube = 3,
    hhBoth = 4
} HH_STATUS;

typedef enum
{
    Standby,
    Treatment,
    Calibration,
    RunMaskFit
} OPERATION_MODE;

typedef enum
{
    RampStart,
    RampStop
} SYSTEM_STATE;

typedef struct
{
    int RampTime;     // unit:minute
    float RampStartP; // 4cmh2o -> 40
    float TherapyP;   // 4cmh2o -> 40
    // float LeakP;
    int PVALevel;
    // float SwingP;
    float MinP; // 4cmh2o -> 40
    float MaxP; // 4cmh2o -> 40
    int Mode;   // 0: CPAP
} Configs;

struct Systems
{
    // int State; //0: ramp on, 1: treatment mode, 2: ramp off
    // int Counter;
    uint32_t Counter10ms;
    uint8_t OperationMode;
    float TargetP;
    float RecordTargetP;
    float SensorP;
    float FinalP;
    float LeakP;
    float SwingP;
    float PVAP;
    // uint32_t TargetSRPM;
    // uint32_t SensorRPM;
    unsigned int T_RPM; // target rpm
    unsigned int S_RPM; // sensor(now) rpm
    // uint32_t Stable_count;
    // add in 20191008
    uint16_t Leakage;
    uint16_t LargeLeakTime;
    // uint16_t WithoutTubingCounter;
    int BreathPhaseStatus;
    uint8_t IsDuringRamp;
    // int PVA_setP;
    uint16_t BreathFrequency;
    Configs Config;
    uint32_t TotalLeakage;
};

typedef struct
{
    unsigned int Flow_ADC;
    unsigned int Press_ADC;
    // unsigned int FlowOffSet; // 32771
    float FlowLPM; // = 0;
    // float FlowP;
    //  uint16_t Averaged;           // = 0;
    // uint16_t LastedLPM;  // = 0
    float FlowLPM_M0; // = 0
    float FlowLPM_M1; // = 0
    //  uint32_t LeakAveraged;       // = 0;
    //  uint32_t LastedLeakBaseline; // = 1000000;
    //   int16_t OffSetTo1000;        // = 0
    float LeakBaseline; // = 0
    float RoughBaseline;
    // uint8_t CheckPhase;
    // uint8_t BreathPhase;

    // uint16_t Fix_time;
    // uint16_t Rough_time;
    // uint8_t flag;
    //  uint32_t Swing_sum;
    //  uint16_t Swing_baseline;
    //  uint16_t Swing_index;
    uint8_t BreathFrequency;

} structPQCurve;

// extern _PQ_Curve PQ_Curve;

typedef struct
{
    int8_t HALL;
    int8_t Chamber;
    int8_t HLV;
    int8_t Tube;
    int8_t TLV;
    int8_t ON;
} HH_STATUS_T;
extern HH_STATUS_T HHStatus;

// this is for graphic
typedef union
{
    uint8_t bytes[20];
    struct
    {
        uint32_t version;
        uint32_t idx_checksum;
        uint32_t data_checksum;
        uint32_t idx_length;
        uint32_t data_length;
    } Items;
} ValidationOfGraphic_t;

typedef enum
{
    GR_OK = 0,
    GR_NoSD,
    GR_FileOpenErr,
    GR_FileReadErr,
    GR_HeaderErr,
    GR_VerErr,
    GR_MagicErr,
    GR_BmpNumErr,
    GR_Match,
    GR_InfoErr,
    GR_IndexErr,
    GR_DataErr,
    GR_BodyErr,
    GR_BodyOK
} GraphReadResult_t;

typedef enum
{
    GR_UpdatedOK = 0,
    GR_UpdatedIgnore,
    GR_UpdatedHeaderErr,
    GR_UpdatedBodyErr,
    GR_UpdatedWriteFail,
} GraphUpdateResult_t;

// for graphic loading "v2"
typedef union __IMAGE_HEADER__
{
    uint8_t Bytes[24];
    struct
    {
        char Magic[4];
        unsigned int Version;
        unsigned int HeaderChecksum;
        unsigned int OffsetChecksum;
        unsigned int DataChecksum;
        unsigned int TotalImageSize;
        unsigned int Numbers;
    } Items;
} ImageHeader_t;

typedef union __IMAGE_OFFSET__
{
    uint8_t Bytes[8];
    struct
    {
        unsigned int Offset;
        unsigned int DataSize;
    } Items;
} ImageOffset_t;

typedef struct __IMAGE_DATA__
{
    unsigned int DataSize;
    unsigned char *Data;
} ImageData_t;

#endif
