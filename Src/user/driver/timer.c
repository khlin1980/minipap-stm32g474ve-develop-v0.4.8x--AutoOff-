/**========================================================================
 *                           includes
 *========================================================================**/
#include "timer.h"

/**========================================================================
 *                           typedef
 *========================================================================**/

/**========================================================================
 *                           define
 *========================================================================**/
#define TIMER_COUNTING_BASE_us      500                
#define TIMER_BASE_ms(x)            (x)           
#define TIMER_BASE_sec(x)           (x * 1000)    

#define TIMER_COUNT_1ms     (uint16_t)((1 * 1000) / TIMER_COUNTING_BASE_us) 
#define TIMER_COUNT_5ms     (uint16_t)(5 / TIMER_BASE_ms(1))    
#define TIMER_COUNT_10ms    (uint16_t)(10 / TIMER_BASE_ms(1))    
#define TIMER_COUNT_100ms   (uint16_t)(100/ TIMER_BASE_ms(10))    
#define TIMER_COUNT_1s      (uint16_t)(1000 / TIMER_BASE_ms(100))    
#define TIMER_COUNT_1m      (uint16_t)(60 * 1000 / TIMER_BASE_sec(1))              


/**========================================================================
 *                           macro
 *========================================================================**/

/**========================================================================
 *                           variables
 *========================================================================**/
volatile uint16_t tmr_base_flag = 0;
uint16_t tmr_5ms_flag = 0; 
uint16_t tmr_10ms_flag = 0; 
uint16_t tmr_100ms_flag = 0; 
uint16_t tmr_1s_flag = 0; 

uint16_t tmrcnt_1ms = 0;
uint16_t tmrcnt_5ms = 0;
uint16_t tmrcnt_10ms = 0;
uint16_t tmrcnt_100ms = 0;
uint16_t tmrcnt_1s = 0;

uint16_t tmr_routine = 0;

/**========================================================================
 *                           functions
 *========================================================================**/
void TimerInit(void);
void TimerInc(void);
void TimeScan(void);


/**
 * @brief 
 * 
 */
void TimerInit(void)
{
  tmr_base_flag = 0;
  tmr_5ms_flag = 0; 
  tmr_10ms_flag = 0; 
  tmr_100ms_flag = 0; 
  tmr_1s_flag = 0; 
  
  tmrcnt_1ms = 0;
  tmrcnt_5ms = 0;
  tmrcnt_10ms = 0;
  tmrcnt_100ms = 0;
  tmrcnt_1s = 0;

  tmr_routine = 0;
}

/**
 * @brief 
 * 
 */
void TimerInc(void)
{
  tmr_base_flag++;
}

/**
 * @brief software timer service routine
 * 
 */
void TimeScan(void)
{
  tmr_routine = TMR_ROUTINE_None; 
  
  if(tmr_base_flag)
  {
    tmr_base_flag = 0;
    
    /**========================================================================
    *?                           1ms routine
    *========================================================================**/
    if(++tmrcnt_1ms >= TIMER_COUNT_1ms)
      tmrcnt_1ms = 0;

    if(tmrcnt_1ms == 0)
      tmr_5ms_flag = 1;
    else
      tmr_10ms_flag = 1;

    /**========================================================================
    *?                           5ms routine
    *========================================================================**/
    if(tmr_5ms_flag)
    {
      tmr_5ms_flag = 0;
      
      if(++tmrcnt_5ms >= TIMER_COUNT_5ms)
        tmrcnt_5ms = 0;

      tmr_routine = (uint16_t)(TMR_ROUTINE_5ms_0 + tmrcnt_5ms);   
    }

    /**========================================================================
    *?                           10ms routine
    *========================================================================**/
    if(tmr_10ms_flag)
    {
      tmr_10ms_flag = 0;
      
      if(++tmrcnt_10ms >= TIMER_COUNT_10ms)
        tmrcnt_10ms = 0;

      if(tmrcnt_10ms == 3)
        tmr_100ms_flag = 1;
      else  
        tmr_routine = (uint16_t)(TMR_ROUTINE_10ms_0 + tmrcnt_10ms);   
    }

    /**========================================================================
    *?                           100ms routine
    *========================================================================**/
    if(tmr_100ms_flag)
    {
      tmr_100ms_flag = 0;
      
      if(++tmrcnt_100ms >= TIMER_COUNT_100ms)
        tmrcnt_100ms = 0;

      if(tmrcnt_100ms == 5)
        tmr_1s_flag = 1;
      else  
        tmr_routine = (uint16_t)(TMR_ROUTINE_100ms_0 + tmrcnt_100ms);   
    }

    /**========================================================================
    *?                           1 second routine
    *========================================================================**/
    if(tmr_1s_flag)
    {
      tmr_1s_flag = 0;
      
      if(++tmrcnt_1s >= TIMER_COUNT_1s)
        tmrcnt_1s = 0;

      tmr_routine = (uint16_t)(TMR_ROUTINE_1s_0 + tmrcnt_1s);   
    }
  }
}
