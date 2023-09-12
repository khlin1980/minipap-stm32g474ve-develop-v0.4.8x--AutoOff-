/**========================================================================
 *                           includes
 *========================================================================**/
#include "debug_io.h"

/**========================================================================
 *                           typedef
 *========================================================================**/

/**========================================================================
 *                           define
 *========================================================================**/
#define DEBUG_IO_PORT   GPIOD
#define DEBUG_IO_PIN    GPIO_PIN_1

/**========================================================================
 *                           macro
 *========================================================================**/

/**========================================================================
 *                           variables
 *========================================================================**/

/**========================================================================
 *                           functions
 *========================================================================**/


/**
 * @brief 
 * 
 */
void Debug_IO_Toggle(void)
{
 #if(USE_DEBUG_IO == 1) 
  HAL_GPIO_TogglePin(DEBUG_IO_PORT, DEBUG_IO_PIN);
 #endif 
}


/**
 * @brief 
 * 
 */
void Debug_IO_Set(uint8_t lv)
{
 #if(USE_DEBUG_IO == 1) 
  if(lv)
    HAL_GPIO_WritePin(DEBUG_IO_PORT, DEBUG_IO_PIN, GPIO_PIN_SET);
  else  
    HAL_GPIO_WritePin(DEBUG_IO_PORT, DEBUG_IO_PIN, GPIO_PIN_RESET);
 #endif 
}


/**
 * @brief 
 * 
 */
void print_debug_msg(const char *fmt, ...) 
{
 #if(USE_DEBUG_PRINT == 1)
  
  static char buffer[512];
  va_list args;
  va_start(args, fmt);
  vsnprintf(buffer, sizeof(buffer), fmt, args);
  va_end(args);

  int len = strlen(buffer);
  //HAL_UART_Transmit(&huart2, (uint8_t*)buffer, len, -1);
  
  printf("%s", buffer);
 
 #endif 
}

