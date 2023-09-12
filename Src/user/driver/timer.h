#ifndef __TIMER_H
#define __TIMER_H

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
typedef enum
{
  TMR_ROUTINE_None = 0,
  
  //* 1ms (counting base 500us)
  TMR_ROUTINE_1ms_0,
  TMR_ROUTINE_1ms_1,
  
  //* 5ms (counting base 1ms)
  TMR_ROUTINE_5ms_0,
  TMR_ROUTINE_5ms_1,
  TMR_ROUTINE_5ms_2,
  TMR_ROUTINE_5ms_3,
  TMR_ROUTINE_5ms_4,

  //* 10ms (counting base 1ms)
  TMR_ROUTINE_10ms_0,
  TMR_ROUTINE_10ms_1,
  TMR_ROUTINE_10ms_2,
  TMR_ROUTINE_10ms_3,
  TMR_ROUTINE_10ms_4,
  TMR_ROUTINE_10ms_5,
  TMR_ROUTINE_10ms_6,
  TMR_ROUTINE_10ms_7,
  TMR_ROUTINE_10ms_8,
  TMR_ROUTINE_10ms_9,

  //* 100ms (counting base 10ms)
  TMR_ROUTINE_100ms_0,
  TMR_ROUTINE_100ms_1,
  TMR_ROUTINE_100ms_2,
  TMR_ROUTINE_100ms_3,
  TMR_ROUTINE_100ms_4,
  TMR_ROUTINE_100ms_5,
  TMR_ROUTINE_100ms_6,
  TMR_ROUTINE_100ms_7,
  TMR_ROUTINE_100ms_8,
  TMR_ROUTINE_100ms_9,

  //* 1s (counting base 100ms)
  TMR_ROUTINE_1s_0,
  TMR_ROUTINE_1s_1,
  TMR_ROUTINE_1s_2,
  TMR_ROUTINE_1s_3,
  TMR_ROUTINE_1s_4,
  TMR_ROUTINE_1s_5,
  TMR_ROUTINE_1s_6,
  TMR_ROUTINE_1s_7,
  TMR_ROUTINE_1s_8,
  TMR_ROUTINE_1s_9,

  //* end of routine
  TMR_ROUTINE_END = 0xffff 

} tmr_rontine_e;


/**========================================================================
 *                           exported constants
 *========================================================================**/

/**========================================================================
 *                           exported macro
 *========================================================================**/

/**========================================================================
 *                           exported variables
 *========================================================================**/
extern uint16_t tmr_routine;

/**========================================================================
 *                           exported functions
 *========================================================================**/
void TimerInit(void);
void TimerInc(void);
void TimeScan(void);

#ifdef __cplusplus
}
#endif

#endif