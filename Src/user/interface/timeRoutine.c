/**========================================================================
 *                           includes
 *========================================================================**/
#include "timeRoutine.h"

// driver
#include "debug_io.h"
#include "key_fifo.h"
#include "led.h"

// interface
#include "sensor_if.h"
#include "rtc.h"    

//system
#include "regular_adc.h"


/**========================================================================
 *                           typedef
 *========================================================================**/

/**========================================================================
 *                           define
 *========================================================================**/
          
/**========================================================================
 *                           macro
 *========================================================================**/

/**========================================================================
 *                           variables
 *========================================================================**/
uint16_t roughcnt_10ms = 0;
uint16_t roughcnt_100ms = 0;
uint16_t roughcnt_1s = 0;

/**========================================================================
 *                           functions
 *========================================================================**/
void TimeService(void);
void RoughTimer(void);


/**
 * @brief 
 * 
 */
void TimeService(void)
{
  switch(tmr_routine)
  {
    /**========================================================================
    *?                           1ms routine (1/0.5)
    *========================================================================**/
    case TMR_ROUTINE_1ms_0:
    break;

    case TMR_ROUTINE_1ms_1:
    break;

    /**========================================================================
    *?                           5ms routine (5/1)
    *========================================================================**/
    case TMR_ROUTINE_5ms_0:
    RoughTimer();
    break;

    case TMR_ROUTINE_5ms_1:
    //Debug_IO_Toggle();
    break;

    case TMR_ROUTINE_5ms_2:
    sensors_control_polling();
    break;

    case TMR_ROUTINE_5ms_3:
    break;

    case TMR_ROUTINE_5ms_4:
    break;

    /**========================================================================
    *?                           10ms routine (10/1)
    *========================================================================**/
    case TMR_ROUTINE_10ms_0:
    KeyScan10ms(); 
    break;

    case TMR_ROUTINE_10ms_1:
    led_control();
    break;

    case TMR_ROUTINE_10ms_2:
    break;

    case TMR_ROUTINE_10ms_3:
    // do not use this, will never call (for 100ms couting) 
    break;

    case TMR_ROUTINE_10ms_4:
    break;

    case TMR_ROUTINE_10ms_5:
    break;

    case TMR_ROUTINE_10ms_6:
    break;

    case TMR_ROUTINE_10ms_7:
    break;

    case TMR_ROUTINE_10ms_8:
    break;

    case TMR_ROUTINE_10ms_9:
    break;

    /**========================================================================
    *?                           100ms routine (100/10)
    *========================================================================**/
    case TMR_ROUTINE_100ms_0:
    user_regular_adc_measure_requirement();
    break;

    case TMR_ROUTINE_100ms_1:
    break;

    case TMR_ROUTINE_100ms_2:
    break;

    case TMR_ROUTINE_100ms_3:
    break;

    case TMR_ROUTINE_100ms_4:
    break;

    case TMR_ROUTINE_100ms_5:
    // do not use this, will never call (for 1s couting) 
    break;

    case TMR_ROUTINE_100ms_6:
    break;

    case TMR_ROUTINE_100ms_7:
    break;

    case TMR_ROUTINE_100ms_8:
    break;

    case TMR_ROUTINE_100ms_9:
    break;

    /**========================================================================
    *?                           1s routine (1000/100)
    *========================================================================**/
    case TMR_ROUTINE_1s_0:
    //* decrease read rtc freq is better, recommend read rtc per minute or hour
    get_rtc(&g_rtc_date, &g_rtc_time);
    break;

    case TMR_ROUTINE_1s_1:
    break;

    case TMR_ROUTINE_1s_2:
    break;

    case TMR_ROUTINE_1s_3:
    break;

    case TMR_ROUTINE_1s_4:
    break;

    case TMR_ROUTINE_1s_5:
    break;

    case TMR_ROUTINE_1s_6:
    break;

    case TMR_ROUTINE_1s_7:
    break;

    case TMR_ROUTINE_1s_8:
    break;

    case TMR_ROUTINE_1s_9:
    break;

    default:
      // usually, do no thing
    break;
  }
}

/**
 * @brief for general purpose use (not accurate, software timer)
 * 
 */
void RoughTimer(void)
{
  static uint16_t rough_tmr_base_5ms = 0;
  static uint16_t rough_tmr_base_10ms = 0;
  static uint16_t rough_tmr_base_100ms = 0;
  
  // increase per 5ms
  if(++rough_tmr_base_5ms >=2)
  {
    rough_tmr_base_5ms = 0;

    //* for 10ms counting
    roughcnt_10ms++;
    //! here, put code what you want
    // ex: usr_device_10ms_cnt++;

    if(++rough_tmr_base_10ms >= 10)
    {
      rough_tmr_base_10ms = 0;
      
      //* for 100ms counting 
      roughcnt_100ms++;
      //! here, put code what you want

      if(++rough_tmr_base_100ms >= 10)
      {
        rough_tmr_base_100ms = 0;
        
        //* for 1s counting 
        roughcnt_1s++;
        //! here, put code what you want
        
      }
    }
  }
}

