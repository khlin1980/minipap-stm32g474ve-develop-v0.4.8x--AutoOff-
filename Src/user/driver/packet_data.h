#ifndef __PACKET_DATA_H
#define __PACKET_DATA_H

#ifdef __cplusplus
 extern "C" {
#endif

/**========================================================================
 *                           includes
 *========================================================================**/
#include "bsp.h"

/**========================================================================
 *                           exported types
 *========================================================================**/

typedef struct _packet_info_
{
    uint8_t ver;            /* verion */ 
    uint16_t dataSize;      /* number of data of structure */
}packet_info_t;


typedef struct
{
    const uint8_t fwVer[10];       /* firmware verion */ 
    uint8_t SN[20];         /* serial number */ 
    uint8_t modelName[20];   
}device_info_t;


typedef struct
{
    const uint8_t fwVer[10];       /* firmware verion */ 
    uint8_t bt_address[6];  /* address */ 
}BLE_info_t;


typedef struct
{
    const uint8_t fwVer[20];      /* firmware verion */  
}MCSDK_info_t;


/**
 * @brief system information structure
 * 
 */
typedef struct
{
    device_info_t device;   /* device information */
    BLE_info_t    BLE;      /* BLE device information */
    MCSDK_info_t  MC;       /* motor control information */

}infos_t;


typedef struct
{
    bool enable;
    bool active;
    uint16_t ramp_time_sec;
    uint16_t ramp_cnt_sec;
}ramp_ctrl_t;


typedef struct
{
    bool enable;
    
}BLE_ctrl_t;


typedef struct
{
    bool autoOnEnable;
    bool autoOffEnable;
    bool autoOnTriggered;
    bool autoOffTriggered;
    
}smart_ctrl_t;


typedef struct
{
    uint64_t utc;
    
}rtc_ctrl_t;


typedef struct
{
    uint8_t maxPressure;
    
}limitation_ctrl_t;


typedef struct
{
    rtc_ctrl_t   rtc_ctrl;              
    ramp_ctrl_t  ramp_ctrl;
    BLE_ctrl_t   BLE_ctrl;
    smart_ctrl_t smart_ctrl;
    limitation_ctrl_t limitation_ctrl;
       
}configs_t;


typedef struct
{
    uint8_t CA_mode;            /* CPAP, APAP mode */
    uint8_t ramp_time_mins;     /* ramp setting time (minutes) */
    uint8_t ramp_p;             /* ramp startup pressure */
    uint8_t target_p;           /* therapy pressure - cpap mode */
    uint8_t apap_init_p;        /* apap therapy init pressure */
    uint8_t apap_max_p;         /* apap therapy max pressure */
    uint8_t apap_min_p;         /* apap therapy min pressure */
    uint8_t pva_lv;             /* pressure variation adjustment level */
    uint8_t checksum;           
    
}prescription_t;


typedef union
{
    uint32_t u32code;
    struct
    {
        uint32_t queue      : 27;     /* event queue, 3bits a menber * 9 */ 
        uint32_t counter    : 5;      /* event counter */
    }str;  

}event_record_t;


typedef struct
{
    //! checksum is must be in 1st member, do not move
    uint16_t checksum;                  /* checksum */ 
    uint8_t now_setting_p;              /* current setting pressure in 1 minute */
    uint8_t now_leak;                   /* current leak(LPM) in 1 minute  */
    event_record_t  evt_record;         /* current event statistics in 1 minute*/
    
}therapy_rec_data_t;


typedef struct
{
  //! checksum is must be in 1st member, do not move
  uint16_t checksum;                /* checksum */ 
  uint32_t No;                      /* serial number */
  uint64_t occurredTime_utc;        /* occurred time (UTC format)*/
  union{
    uint32_t faultCode;             /* fault code */
    uint32_t fileName;              /* file name (only number) */
  }type;

}log_rec_data_t;


typedef struct
{
    packet_info_t infos;
    packet_info_t configs;
    packet_info_t prescription;
    packet_info_t therapy_rec_data;
    packet_info_t fault_rec_data;

}packet_infos_t;


typedef struct
{
    infos_t         infos;          /* system devices information */      
    configs_t       configs;        /* system devices configuration*/
    prescription_t  prescription;   /* prescription of therapy */
    packet_infos_t  packet_infos;   /* record the relative structures version */

}sys_vars_t;


/**========================================================================
 *                           exported constants
 *========================================================================**/

/**========================================================================
 *                           exported macro
 *========================================================================**/

/**========================================================================
 *                           exported variables
 *========================================================================**/

/**========================================================================
 *                           exported functions
 *========================================================================**/

#ifdef __cplusplus
}
#endif

#endif